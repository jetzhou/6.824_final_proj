// this is the extent server

#ifndef extent_server_h
#define extent_server_h

#include <string>
#include <map>
#include "extent_protocol.h"

class extent_server {
  // stores the attributes of each file
  std::map<extent_protocol::extentid_t, extent_protocol::attr> attrs;
  // stores the contents of each file
  std::map<extent_protocol::extentid_t, std::string> contents;
  // stores keys for userids 
  std::map<extent_protocol::userid_t, std::string> user_keys;
  //stores list of userids for each group id
  std::map<extent_protocol::groupid_t, std::list<extent_protocol::userid_t> > groupusers;
  //stores groupids for group names
  std::map<std::string, extent_protocol::groupid_t> groupids;
  
  
 private:
  pthread_mutex_t mutex;
  extent_protocol::groupid_t group_num; //keeps track of number of groups
  
  bool has_read_perm(extent_protocol::extentid_t, extent_protocol::userid_t);
  bool has_write_perm(extent_protocol::extentid_t, extent_protocol::userid_t);
  bool has_execute_perm(extent_protocol::extentid_t, extent_protocol::userid_t);
  bool in_group(extent_protocol::userid_t, std::string);
  bool in_group(extent_protocol::userid_t, extent_protocol::groupid_t);
  bool group_exists(std::string);

 public:
  extent_server();

  int put(extent_protocol::extentid_t id, std::string, extent_protocol::userid_t, std::string, int &);
  int get(extent_protocol::extentid_t id, extent_protocol::userid_t, std::string, std::string &);
  int getattr(extent_protocol::extentid_t id, extent_protocol::userid_t, std::string, extent_protocol::attr &);
  int setattr(extent_protocol::extentid_t id, extent_protocol::attr a, extent_protocol::userid_t, std::string, int &);
  int remove(extent_protocol::extentid_t id, extent_protocol::userid_t, std::string, int &);
  
  int reg(extent_protocol::userid_t userid, std::string userkey, int &);
  
  int add_group(std::string);
  int add_user(extent_protocol::userid_t);
  int add_user_to_group(extent_protocol::userid_t, std::string);

};

#endif 







