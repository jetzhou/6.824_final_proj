// extent wire protocol

#ifndef extent_protocol_h
#define extent_protocol_h

#include "rpc.h"

class extent_protocol {
 public:
  typedef int status;
  typedef unsigned long long extentid_t; //extentid
  typedef unsigned int userid_t; //userid
  typedef unsigned int groupid_t; //groupid
  
  enum xxstatus { OK, RPCERR, NOENT, IOERR, NOACCESS };
  enum rpc_numbers {
    put = 0x6001,
    get,
    getattr,
    remove,
    setattr,
    reg,
    groupadd,
    groupdel,
    useradd,
    userdel
  };

  struct attr {
    unsigned int atime;
    unsigned int mtime;
    unsigned int ctime;
    unsigned int size;
    unsigned int mode;
    extent_protocol::userid_t uid;
    extent_protocol::groupid_t gid;
  };
};

inline unmarshall &
operator>>(unmarshall &u, extent_protocol::attr &a)
{
  u >> a.atime;
  u >> a.mtime;
  u >> a.ctime;
  u >> a.size;
  u >> a.mode;
  u >> a.uid;
  u >> a.gid;
  return u;
}

inline marshall &
operator<<(marshall &m, extent_protocol::attr a)
{
  m << a.atime;
  m << a.mtime;
  m << a.ctime;
  m << a.size;
  m << a.mode;
  m << a.uid;
  m << a.gid;
  return m;
}

#endif 
