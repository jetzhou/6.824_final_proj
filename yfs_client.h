#ifndef yfs_client_h
#define yfs_client_h

#include <string>
//#include "yfs_protocol.h"
#include "extent_client.h"
#include <vector>

#include "lock_protocol.h"
#include "lock_client.h"

class yfs_client {
  extent_client *ec;
  lock_client *lc;
 
 public:

  typedef unsigned long long inum;
  enum xxstatus { OK, RPCERR, NOENT, IOERR, EXIST, NOACCESS };
  typedef int status;

  struct fileinfo {
    unsigned long long size;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
    unsigned long mode;
    unsigned long uid;
    unsigned long gid;
  };
  struct dirinfo {
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
    unsigned long mode;
    unsigned long uid;
    unsigned long gid;
  };
  struct dirent {
    std::string name;
    yfs_client::inum inum;
  };

 private:
  static std::string filename(inum);
  static inum n2i(std::string);
 public:

  yfs_client(std::string, std::string, std::string);

  bool isfile(inum);
  bool isdir(inum);

  int getfile(inum, fileinfo &);
  int getdir(inum, dirinfo &);
  int lookup(inum, std::string, inum &);
  int create(inum, inum, std::string, mode_t);
  int unlink(inum, std::string);
  int readdir(inum, std::list<dirent> &);
  int write(inum, off_t, std::string);
  int read(inum, off_t, size_t, std::string &);
  int truncate(inum, size_t);
  int chmod(inum, unsigned long);
  int chown(inum, extent_protocol::userid_t, extent_protocol::groupid_t);
};

#endif 
