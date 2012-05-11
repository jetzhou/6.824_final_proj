// extent client interface.

#ifndef extent_client_h
#define extent_client_h

#include <string>
#include "extent_protocol.h"
#include "rpc.h"
#include <sys/types.h>

class extent_client {
 private:
  rpcc *cl;
  extent_protocol::userid_t userid;
  std::string userkey;
  

 public:
  extent_client(std::string dst, extent_protocol::userid_t, std::string);

  extent_protocol::status get(extent_protocol::extentid_t eid, 
			      std::string &buf);
  extent_protocol::status getattr(extent_protocol::extentid_t eid, 
				  extent_protocol::attr &a);
  extent_protocol::status put(extent_protocol::extentid_t eid, std::string buf);
  extent_protocol::status remove(extent_protocol::extentid_t eid);
  extent_protocol::status setmode(extent_protocol::extentid_t eid,
                                  extent_protocol::attr a);
};

#endif 

