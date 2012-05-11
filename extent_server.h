// this is the extent server

#ifndef extent_server_h
#define extent_server_h

#include <string>
#include <map>
#include "extent_protocol.h"

class extent_server {

 private:
  pthread_mutex_t mutex;

 public:
  extent_server();

  // stores the attributes of each file
  std::map<extent_protocol::extentid_t, extent_protocol::attr> attrs;
  // stores the contents of each file
  std::map<extent_protocol::extentid_t, std::string> contents;
  
  // stores keys for userids 
  std::map<extent_protocol::userid_t, std::string> user_keys;
  //stores list of userids for each group id
  std::map<extent_protocol::group, std::list<extent_protocol::userid_t> > group_users;
  
  int put(extent_protocol::extentid_t id, std::string, extent_protocol::userid_t, std::string, int &);
  int get(extent_protocol::extentid_t id, extent_protocol::userid_t, std::string, std::string &);
  int getattr(extent_protocol::extentid_t id, extent_protocol::userid_t, std::string, extent_protocol::attr &);
  int setattr(extent_protocol::extentid_t id, extent_protocol::attr a, extent_protocol::userid_t, std::string, int &);
  int remove(extent_protocol::extentid_t id, extent_protocol::userid_t, std::string, int &);
};

#endif 







