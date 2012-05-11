// this is the extent server

#ifndef extent_server_h
#define extent_server_h

#include <string>
#include <map>
#include "extent_protocol.h"

class extent_server {
  // stores keys for userids 
  std::map<extent_protocol::userid_t, std::string> user_keys;
  //stores list of userids for each group id
  std::map<extent_protocol::groupid_t, std::list<extent_protocol::userid_t> > groupusers;
  //stores groupids for group names
  std::map<std::string, extent_protocol::groupid_t> groupids;
  
 private:
  pthread_mutex_t mutex;
  
  bool has_read_perm(extent_protocol::extentid_t, extent_protocol::userid_t);
  bool has_write_perm(extent_protocol::extentid_t, extent_protocol::userid_t);
  bool has_execute_perm(extent_protocol::extentid_t, extent_protocol::userid_t);
  bool in_group(extent_protocol::userid_t, std::string);
  bool in_group(extent_protocol::userid_t, extent_protocol::groupid_t);

 public:
  extent_server();

  // stores the attributes of each file
  std::map<extent_protocol::extentid_t, extent_protocol::attr> attrs;
  // stores the contents of each file
  std::map<extent_protocol::extentid_t, std::string> contents;

  
  int put(extent_protocol::extentid_t id, std::string, extent_protocol::userid_t, std::string, int &);
  int get(extent_protocol::extentid_t id, extent_protocol::userid_t, std::string, std::string &);
  int getattr(extent_protocol::extentid_t id, extent_protocol::userid_t, std::string, extent_protocol::attr &);
  int setmode(extent_protocol::extentid_t id, extent_protocol::attr a, extent_protocol::userid_t, std::string, int &);
  int remove(extent_protocol::extentid_t id, extent_protocol::userid_t, std::string, int &);

};

#endif 







