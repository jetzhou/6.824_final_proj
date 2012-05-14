#ifndef group_util_h
#define group_util_h

#include <string>

#include "rpc.h"
#include "extent_protocol.h"

class group_util {
  private:
    rpcc* cl;

  public:
    group_util(std::string);

    int groupadd(extent_protocol::groupid_t, extent_protocol::userid_t,
            std::string);
    int groupdel(extent_protocol::groupid_t, extent_protocol::userid_t,
            std::string);
    int useradd(extent_protocol::userid_t, extent_protocol::groupid_t,
            extent_protocol::userid_t, std::string);
    int userdel(extent_protocol::userid_t, extent_protocol::groupid_t,
            extent_protocol::userid_t, std::string);

};

#endif
