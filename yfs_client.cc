// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include "lock_client.h"
#include "server_slock.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/*
 * split a std::string by a delimiter
 * taken from:
 * http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
 */
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}


yfs_client::yfs_client(std::string extent_dst, std::string lock_dst, std::string client_key)
{
  ec = new extent_client(extent_dst);
  lc = new lock_client(lock_dst);
  // put in root directory
  inum root = 0x80000001;
  ServerScopedLock sl(lc, (lock_protocol::lockid_t) root);
  ec->put(root, "");
}

yfs_client::inum
yfs_client::n2i(std::string n)
{
  std::istringstream ist(n);
  unsigned long long finum;
  ist >> finum;
  return finum;
}

std::string
yfs_client::filename(inum inum)
{
  std::ostringstream ost;
  ost << inum;
  return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
  if(inum & 0x80000000)
    return true;
  return false;
}

bool
yfs_client::isdir(inum inum)
{
  return ! isfile(inum);
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
  int r = OK;
  // You modify this function for Lab 3
  // - hold and release the file lock

  printf("getfile %016llx\n", inum);
  extent_protocol::attr a;
  
  ServerScopedLock sl(lc, (lock_protocol::lockid_t) inum);

  if (ec->getattr(inum, a) != extent_protocol::OK) {
    return IOERR;
  }

  fin.atime = a.atime;
  fin.mtime = a.mtime;
  fin.ctime = a.ctime;
  fin.size = a.size;
  printf("getfile %016llx -> sz %llu\n", inum, fin.size);

  return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
  int r = OK;
  // You modify this function for Lab 3
  // - hold and release the directory lock

  printf("getdir %016llx\n", inum);
  extent_protocol::attr a;
  
  ServerScopedLock sl(lc, (lock_protocol::lockid_t) inum);

  if (ec->getattr(inum, a) != extent_protocol::OK) {
    return IOERR;
  }
  din.atime = a.atime;
  din.mtime = a.mtime;
  din.ctime = a.ctime;

  return r;
}

int
yfs_client::lookup(inum pnum, std::string fname, inum &fnum)
{
  ServerScopedLock sl(lc, (lock_protocol::lockid_t) pnum);

  // get the associated content with this inum
  std::string buf;
  ec->get(pnum, buf);

  // parse the string
  std::vector<std::string> name_n_nums = split(buf, '/');

  // find the fname
  std::vector<std::string>::iterator it;
  it = std::find(name_n_nums.begin(), name_n_nums.end(), fname);
  if (it == name_n_nums.end()) {
    fnum = 0;
    return NOENT;
  } else {
    fnum = n2i(*(++it));
    return OK;
  }
}

int
yfs_client::create(inum pnum, inum fnum, std::string fname, mode_t mode)
{
  ServerScopedLock psl(lc, (lock_protocol::lockid_t) pnum);
  ServerScopedLock fsl(lc, (lock_protocol::lockid_t) fnum);

  // get the directory string first
  std::string buf;
  ec->get(pnum, buf);

  // see if the name already exists
  if (buf.find(fname) != std::string::npos) {
    return EXIST;
  } else {
    // add this new pair of fnum and fname to the end, save it
    buf += fname + "/" + filename(fnum) + "/";
    ec->put(pnum, buf);
 
    // save a new record for the fnum, empty string as its content
    ec->put(fnum, "");
    extent_protocol::attr a;
    ec->getattr(fnum, a);
    a.mode = mode;
    ec->setmode(fnum, a);
    return OK;
  }
}

int
yfs_client::readdir(inum num, std::list<dirent> &dirents)
{
  ServerScopedLock sl(lc, (lock_protocol::lockid_t) num);
  // get the directory string
  std::string buf;
  ec->get(num, buf); 

  // parse the string
  std::vector<std::string> name_n_nums = split(buf, '/');

  // put each pair of inum and fname into the list
  std::vector<std::string>::iterator it = name_n_nums.begin();
  while (it != name_n_nums.end()) { // technically this should be it+1, no need
                                    // to worry because we always have pairs
    // first one in the pair is fname,
    // second one is inum
    std::string name = *(it++);
    inum n = n2i(*(it++));

    // add to dirents
    dirent ent;
    ent.inum = n;
    ent.name = name;
    dirents.push_back(ent);
  }
  return OK;
}

int
yfs_client::write(inum num, off_t off, std::string buf)
{
  ServerScopedLock sl(lc, (lock_protocol::lockid_t) num);
  
  // get the associated content with this inum
  std::string content;
  ec->get(num, content);
  
  size_t size = content.size();

  // whether offset is within existing content
  if (off < (int) size) {
    content.replace(off, buf.size(), buf);
  } else {
    size_t s = off-size;
    char c = '\0';
    std::string null_str(s, c);
    content.append(null_str);
    content.append(buf);
  }

  // save the content
  ec->put(num, content);

  return OK;
}

int
yfs_client::read(inum num, off_t off, size_t size, std::string &buf)
{
  ServerScopedLock sl(lc, (lock_protocol::lockid_t) num);
  
  // get the associated content with this inum
  std::string content;
  ec->get(num, content);

  buf.replace(0, size, content, off, size);
  return OK;
}

int
yfs_client::truncate(inum num, size_t size)
{
  ServerScopedLock sl(lc, (lock_protocol::lockid_t) num);
  
  // get the associated content with this inum
  std::string content;
  ec->get(num, content);

  size_t trun_len = content.size() - size;
  content.erase(trun_len);

  ec->put(num, content);
  return OK;
}

int
yfs_client::unlink(inum pnum, std::string fname) {
  ServerScopedLock psl(lc, (lock_protocol::lockid_t) pnum);
  
  // first remove the record from its parent
  std::string buf;
  ec->get(pnum, buf);

  // find the fname
  std::vector<std::string> name_n_nums = split(buf, '/');
  std::vector<std::string>::iterator it;
  it = std::find(name_n_nums.begin(), name_n_nums.end(), fname);
  
  // see if it exists at all
  if (it == name_n_nums.end()) {
    return NOENT;
  }

  // otherwise remove the record from parent string
  std::string fnum_str = *(++it);
  size_t npos = fname.size() + 1 + fnum_str.size() + 1; // nameA/1234/nameB/5678/
  buf.erase(buf.find(fname), npos);
  // put it back to the parent record
  ec->put(pnum, buf);
    
  // call remove on server to remove the file
  inum fnum = n2i(fnum_str);
  ServerScopedLock fsl(lc, (lock_protocol::lockid_t) fnum);
  ec->remove(fnum);
  return OK;
}
