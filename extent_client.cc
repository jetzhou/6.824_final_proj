// RPC stubs for clients to talk to extent_server

#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

// The calls assume that the caller holds a lock on the extent

extent_client::extent_client(std::string dst, extent_protocol::userid_t uid, std::string key)
{
  sockaddr_in dstsock;
  userid = uid;
  userkey = key;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() != 0) {
    printf("extent_client: bind failed\n");
  }
}

extent_protocol::status
extent_client::get(extent_protocol::extentid_t eid, std::string &buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::get, eid, buf, userid, userkey);
  return ret;
}

extent_protocol::status
extent_client::getattr(extent_protocol::extentid_t eid, 
		       extent_protocol::attr &attr)
{
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::getattr, eid, attr, userid, userkey);
  return ret;
}

extent_protocol::status
extent_client::setmode(extent_protocol::extentid_t eid, 
		       extent_protocol::attr attr)
{
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::setmode, eid, attr, userid, userkey, r);
  return ret;
}

extent_protocol::status
extent_client::put(extent_protocol::extentid_t eid, std::string buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::put, eid, buf, userid, userkey, r);
  return ret;
}

extent_protocol::status
extent_client::remove(extent_protocol::extentid_t eid)
{
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::remove, eid, userid, userkey, r);
  return ret;
}


