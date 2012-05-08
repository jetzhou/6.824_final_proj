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


int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &)
{
  ScopedLock l(&mutex);
  // create a new attr entry for this extentid
  extent_protocol::attr a;
  time_t cur = time(NULL);
  a.ctime = cur;
  a.mtime = cur;
  if (attrs.find(id) == attrs.end()) {
      a.atime = cur;
  }
  a.size = buf.size();
  attrs[id] = a;

  // saves buf under extentid as its key
  contents[id] = buf;
  return extent_protocol::OK;
}

int extent_server::get(extent_protocol::extentid_t id, std::string &buf)
{
  ScopedLock l(&mutex);
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

int extent_server::getattr(extent_protocol::extentid_t id, extent_protocol::attr &a)
{
  ScopedLock l(&mutex);
  // get attribute correponding to extentid if it exists
  // return NOENT otherwise
  if (contents.count(id) == 0) {
      a.size  = 0;
      a.atime = 0;
      a.mtime = 0;
      a.ctime = 0;
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

int extent_server::setmode(extent_protocol::extentid_t id, extent_protocol::attr a, int &)
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

int extent_server::remove(extent_protocol::extentid_t id, int &)
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

