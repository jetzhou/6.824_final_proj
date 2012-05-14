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
  // put in root directory
  extent_protocol::extentid_t root = 0x00000001;
  contents[root] = "";
  extent_protocol::attr a;
  time_t cur = time(NULL);
  a.atime = cur;
  a.ctime = cur;
  a.mtime = cur;
  a.size = 0;
  a.mode = 0777;
  a.uid = getuid();
  a.gid = getgid();
  attrs[root] = a; 
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
    // for a new record, we also have to initialize its permissions, otherwise
    // the next setattr call will not work. The setattr call is assumed to
    // modify this to the right value immediately
    a.mode = 0777;
  } else {
    a = attrs[id];
  }
  a.ctime = cur;
  a.mtime = cur;
  a.size = buf.size();

  // check permission
  if (!has_write_perm(id, userid, userkey)) {
    return extent_protocol::NOACCESS;
  }

  attrs[id] = a;
  // saves buf under extentid as its key
  contents[id] = buf;

  return extent_protocol::OK;
}

int extent_server::get(extent_protocol::extentid_t id,
    extent_protocol::userid_t userid, std::string userkey, std::string &buf)
{
  ScopedLock l(&mutex);

  if (!has_read_perm(id, userid, userkey)) {
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

  if (!has_write_perm(id, userid, userkey)) {
    return extent_protocol::NOACCESS;
  }

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

  if (!has_write_perm(id, userid, userkey)) {
    return extent_protocol::NOACCESS;
  }

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
  //first time userid is seen by extent server
  if (userkeys.count(userid) == 0) {
    userkeys[userid] = userkey;
  }
  else{
  	if(!authenticate(userid, userkey)){
  		return extent_protocol::NOACCESS;
  	}
  }
  return extent_protocol::OK;
}


//inserts groupid into groupusers map, creates an empty list of users
//Note: this will overwrite preexisting groups with the same gid
int extent_server::groupadd(extent_protocol::groupid_t gid, extent_protocol::userid_t admin, std::string adminkey, int &)
{
  printf("adding group : %u \n", gid);
  //check if admin is actually admin
  if(!isadmin(admin) || !authenticate(admin, adminkey)){
    printf("admin problems - cannot add group %u\n", gid);
    return extent_protocol::NOACCESS;
  }
  
  std::set<extent_protocol::userid_t> temp = groupusers[gid];
  return extent_protocol::OK;
}

//deletes groupid from groupusers map
//Note: this will overwrite preexisting groups with the same gid
int extent_server::groupdel(extent_protocol::groupid_t gid, extent_protocol::userid_t admin, std::string adminkey, int &)
{
  printf("deleting group : %u \n", gid);
  //check if admin is actually admin
  if(!isadmin(admin) || !authenticate(admin, adminkey)){
    printf("admin problems - cannot delete group %u\n", gid);
    return extent_protocol::NOACCESS;
  }

  //group does not exist
  if(groupusers.find(gid) == groupusers.end()){
    printf("cannot delete group %u because it doesn't exist\n", gid);
    return extent_protocol::NOENT;
  }

  //group exists -> erase from map
  groupusers.erase(gid);
  return extent_protocol::OK;
}


//adds userid to groupid
int extent_server::useradd(extent_protocol::userid_t userid, extent_protocol::groupid_t gid, extent_protocol::userid_t admin, std::string adminkey, int &)
{
  printf("adding user %u to group %u \n", userid, gid);
  if (!isadmin(admin) || !authenticate(admin, adminkey)) {
    printf("admin problems - cannot add user %u\n", userid);
    return extent_protocol::NOACCESS;
  } 
  
  groupusers[gid].insert(userid);
  return extent_protocol::OK;
}

//deletes userid from groupid
int extent_server::userdel(extent_protocol::userid_t userid, extent_protocol::groupid_t gid, extent_protocol::userid_t admin, std::string adminkey, int &)
{
  printf("deleting user %u from group %u \n", userid, gid);
  if (!isadmin(admin) || !authenticate(admin, adminkey)) {
    printf("admin problems - cannot delete user %u\n", userid);
    return extent_protocol::OK;
  }

  //group does not exist
  if (groupusers.find(gid) == groupusers.end()) {
    printf("cannot delete user %u from group %u because group doesn't exist\n", userid, gid);
    return extent_protocol::NOENT;
  }

  std::set<extent_protocol::userid_t> users = groupusers[gid];
  //user does not exist
  if(users.find(userid) == users.end()){
    printf("cannot delete user %u from group %u because user doesn't exist\n", userid, gid);
    return extent_protocol::NOENT;
  }

  //user does exist -> remove user
  users.erase(userid);
  groupusers[gid] = users;
  return extent_protocol::OK;
}

//NOTE: the following permissions predicates all called with lock held
//check if user has permission to read extent
bool extent_server::has_read_perm(extent_protocol::extentid_t id, 
    extent_protocol::userid_t userid, std::string userkey)
{
  if(!authenticate(userid, userkey)){
    printf("no read permission - wrong password\n");
    return false;
  }
  // if we have no record of it yet, just return true
  if (attrs.count(id) == 0) {
    return true;
  }
  extent_protocol::attr a = attrs[id];
  return (a.mode&0004) || (a.uid==userid && (a.mode&0400)) ||
    (ingroup(userid, a.gid) && (a.mode&0040));
}

//check if user has permission to write extent
bool extent_server::has_write_perm(extent_protocol::extentid_t id,
    extent_protocol::userid_t userid, std::string userkey)
{
  if(!authenticate(userid, userkey)){
    printf("no write permission - wrong password\n");
    return false;
  }
  // if we have no record of it yet, just return true
  if (attrs.count(id) == 0) {
    return true;
  }
  extent_protocol::attr a = attrs[id];
  return (a.mode&0002) || (a.uid==userid && (a.mode&0200)) ||
    (ingroup(userid, a.gid) && (a.mode&0020));
}

//check if user has permission to execute extent
bool extent_server::has_execute_perm(extent_protocol::extentid_t id,
    extent_protocol::userid_t userid, std::string userkey)
{
  if(!authenticate(userid, userkey)){
    printf("no read permission - wrong password\n");
    return false;
  }
  // if we have no record of it yet, just return true
  if (attrs.count(id) == 0) {
    return true;
  }
  extent_protocol::attr a = attrs[id];
  return (a.mode&0001) || (a.uid==userid && (a.mode&0100)) ||
    (ingroup(userid, a.gid) && (a.mode&0010));
}

bool extent_server::ingroup(extent_protocol::userid_t userid,
    extent_protocol::groupid_t groupid)
{
  std::set<extent_protocol::userid_t> users = groupusers[groupid];
  if(users.find(userid) == users.end()){
    return false;
  }
  return true;
}

//checks if user is admin user
//assumes lock is held
bool extent_server::isadmin(extent_protocol::userid_t userid)
{
  return true;
  //userid NOT admin
  //  if(groupusers[1].find(userid) == groupusers[1].end()){
  //      printf("user %u is not admin. ", userid);
  //      return false;
  //  }
  //  return true;
}

//check if correct userkey (password) is provided for userid
//assumes lock is held
bool extent_server::authenticate(extent_protocol::userid_t userid, std::string userkey)
{
  //return true;
 if(userkeys[userid] == userkey){
     return true;
 }
 else{
     printf("wrong key provided for user %u\n", userid);
     return false;
 }
}


