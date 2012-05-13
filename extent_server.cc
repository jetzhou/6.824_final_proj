// the extent server implementation

#include "extent_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extent_server::extent_server()
{
  pthread_mutex_init(&mutex, NULL);
}


int extent_server::put(extent_protocol::extentid_t id, std::string buf,
    extent_protocol::userid_t userid, std::string userkey, int &)
{
  ScopedLock l(&mutex);
  // create a new attr entry for this extentid
  // if it doesnt exist
  time_t cur = time(NULL);
  extent_protocol::attr a;
  if (attrs.count(id) == 0) {
    a.atime = cur;
  } else {
    a = attrs[id];
  }
  a.ctime = cur;
  a.mtime = cur;
  a.size = buf.size();
  attrs[id] = a;

  // saves buf under extentid as its key
  contents[id] = buf;

  return extent_protocol::OK;
}

int extent_server::get(extent_protocol::extentid_t id,
    extent_protocol::userid_t userid, std::string userkey, std::string &buf)
{
  ScopedLock l(&mutex);

  if (!has_read_perm(id, userid)) {
    printf("extent server thinks no access!\n");
    return extent_protocol::NOACCESS;
  }
  
  // get content corresponding to extentid if it exists
  // return NOENT otherwise
  if (contents.count(id) == 0) {
    return extent_protocol::NOENT;
  } else {
    attrs[id].atime = time(NULL);
    buf = contents[id];
    return extent_protocol::OK;
  }
}

int extent_server::getattr(extent_protocol::extentid_t id, 
    extent_protocol::userid_t userid, std::string userkey, extent_protocol::attr &a)
{
  ScopedLock l(&mutex);
  // get attribute correponding to extentid if it exists
  // return NOENT otherwise
  if (contents.count(id) == 0) {
    a.size  = 0;
    a.atime = 0;
    a.mtime = 0;
    a.ctime = 0;
    a.mode = 0777;
    a.uid = 0;
    a.gid = 0;
    return extent_protocol::NOENT;
  } else {
    extent_protocol::attr attr = attrs[id];
    a.size  = attr.size;
    a.atime = attr.atime;
    a.mtime = attr.mtime;
    a.ctime = attr.ctime;
    a.mode = attr.mode;
    a.uid = attr.uid;
    a.gid = attr.gid;
    return extent_protocol::OK;
  }
}

int extent_server::setattr(extent_protocol::extentid_t id, extent_protocol::attr a,
    extent_protocol::userid_t userid, std::string userkey, int &)
{
  ScopedLock l(&mutex);
  // set mode/uid/gid correponding to extentid if it exists
  // return NOENT otherwise
  if (contents.count(id) == 0) {
    return extent_protocol::NOENT;
  } else {
    extent_protocol::attr attr = attrs[id];
    attr.mode = a.mode;
    attr.uid = a.uid;
    attr.gid = a.gid;
    attrs[id] = attr;
    return extent_protocol::OK;
  }
}

int extent_server::remove(extent_protocol::extentid_t id,
    extent_protocol::userid_t userid, std::string userkey, int &)
{
  ScopedLock l(&mutex);
  // remove extentid entry if it exists
  // return NOENT otherwise
  if (contents.count(id) == 0) {
    return extent_protocol::NOENT;
  } else {
    attrs.erase(id);
    contents.erase(id);
    return extent_protocol::OK;
  }
}

/*
 * called by extent_client upon construction to make sure that the userid
 * userkey association will be made in case it's the first time we are seeing
 * this user
 */
int extent_server::reg(extent_protocol::userid_t userid, std::string userkey, int &)
{
  if (user_keys.count(userid) == 0) {
    user_keys[userid] = userkey;
  }
  return extent_protocol::OK;
}


//makes a group id for the group name
int extent_server::add_group(std::string name)
{
  ScopedLock l(&mutex);
  //can't create group if it already exists
  if(group_exists(name)){
    return extent_protocol::IOERR;
  }
  else{ //generate a groupid for name
    group_num++;

    //add new group to groupids map
    groupids[name] = group_num;
  }
  return extent_protocol::OK;
}



int extent_server::add_user_to_group(extent_protocol::userid_t userid, std::string)
{
  ScopedLock l(&mutex);
  return extent_protocol::OK;
}

int extent_server::add_user(extent_protocol::userid_t userid)
{
  ScopedLock l(&mutex);
  //TODO: check i
  return extent_protocol::OK;
}

//NOTE: the following permissions predicates all called with lock held
//check if user has permission to read extent
bool extent_server::has_read_perm(extent_protocol::extentid_t id, 
    extent_protocol::userid_t userid)
{
  extent_protocol::attr a = attrs[id];
  printf("a.mode = %d, a.uid = %d, userid = %d\n", a.mode, a.uid, userid);
  return (a.mode&0004) || (a.uid == userid && (a.mode&0400)) ||
    (in_group(userid, a.gid) && (a.mode&0040));
}

//check if user has permission to write extent
bool extent_server::has_write_perm(extent_protocol::extentid_t id,
    extent_protocol::userid_t userid)
{
  extent_protocol::attr a = attrs[id];
  return (a.mode&0002) || (a.uid==userid && (a.mode&0200)) ||
    (in_group(userid, a.gid) && (a.mode&0020));
}

//check if user has permission to execute extent
bool extent_server::has_execute_perm(extent_protocol::extentid_t id,
    extent_protocol::userid_t userid)
{
  extent_protocol::attr a = attrs[id];
  return (a.mode&0001) || (a.uid==userid && (a.mode&0100)) ||
    (in_group(userid, a.gid) && (a.mode&0010));
}

//@frango: check if user exists in group
bool extent_server::in_group(extent_protocol::userid_t userid, 
    std::string groupname)
{
  //group does not exist
  if(!group_exists(groupname)){
    return false;
  }

  //group id exists
  extent_protocol::groupid_t groupid = groupids[groupname];
  return in_group(userid, groupid);
}

bool extent_server::in_group(extent_protocol::userid_t userid,
    extent_protocol::groupid_t groupid)
{
  std::list<extent_protocol::userid_t> users = groupusers[groupid];
  std::list<extent_protocol::userid_t>::iterator it;

  for(it = users.begin(); it != users.end(); it++){
    if(*it == userid){
      return true;
    }
  }
  return false;
}

bool extent_server::group_exists(std::string name)
{
  //group id does not exist for group name
  if(groupids.find(name) == groupids.end()){
    printf("extent_server::in_group() group %s does not exist", name.c_str());
    return false;
  }
  return true;

}

