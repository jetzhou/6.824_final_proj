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
  
  int put(extent_protocol::extentid_t id, std::string, int &);
  int get(extent_protocol::extentid_t id, std::string &);
  int getattr(extent_protocol::extentid_t id, extent_protocol::attr &);
  int setmode(extent_protocol::extentid_t id, extent_protocol::attr a, int &);
  int remove(extent_protocol::extentid_t id, int &);
};

#endif 







