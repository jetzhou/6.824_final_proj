#include "group_util.h"
#include <string>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

group_util::group_util(std::string dst) {
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() != 0) {
    printf("group_util: bind failed\n");
  }
}


int
group_util::groupadd(extent_protocol::groupid_t gid, extent_protocol::userid_t adminid, 
        std::string adminkey) {
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::groupadd, gid, adminid, adminkey, r);
  return ret;
}


int
group_util::groupdel(extent_protocol::groupid_t gid, extent_protocol::userid_t adminid,
        std::string adminkey) {
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::groupdel, gid, adminid, adminkey, r);
  return ret;
}


int
group_util::useradd(extent_protocol::userid_t uid, extent_protocol::groupid_t gid, 
        extent_protocol::userid_t adminid, std::string adminkey) {
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::useradd, uid, gid, adminid, adminkey, r);
  return ret;
}

int
group_util::userdel(extent_protocol::userid_t uid, extent_protocol::groupid_t gid,
        extent_protocol::userid_t adminid, std::string adminkey) {
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::userdel, uid, gid, adminid, adminkey, r);
  return ret;
}


int
main(int argc, char *argv[]) {

  if (argc != 7) {
    fprintf(stderr, "Usage: group_util <port-extent-server> <operation> <username> <groupname> <adminname> <adminkey>\n");
    fprintf(stderr, "Possible operations are: groupadd, groupdel, useradd, userdel %d\n", argc);
    exit(1);
  }

  group_util* util = new group_util(argv[1]);

  std::string op(argv[2]);

  char* uname     = argv[3];
  char* gname     = argv[4];
  char* adminname = argv[5];
  std::string adminkey(argv[6]);

  uid_t uid     = (extent_protocol::userid_t) getpwnam(uname)->pw_uid;
  uid_t adminid = (extent_protocol::userid_t) getpwnam(adminname)->pw_uid;
  gid_t gid     = (extent_protocol::groupid_t) getgrnam(gname)->gr_gid;

  extent_protocol::status ret;
  if (op == "groupadd") {
    ret = util->groupadd(gid, adminid, adminkey);
  } else if (op == "groupdel") {
    ret = util->groupdel(gid, adminid, adminkey);
  } else if (op == "useradd") {
    ret = util->useradd(uid, gid, adminid, adminkey);
  } else if (op == "userdel") {
    ret = util->userdel(uid, gid, adminid, adminkey);
  } else {
    fprintf(stderr, "Usage: group_util <port-extent-server> <operation> <username> <groupname> <yourid> <youruserkey>\n");
    fprintf(stderr, "Wrong op provided\n");
    fprintf(stderr, "Possible operations are: groupadd, groupdel, useradd, userdel\n");
    exit(1);
  }

  if (ret == extent_protocol::OK) {
    fprintf(stdout, "%s was successful!\n", op.c_str());
    exit(0);
  } else {
    fprintf(stderr, "%s was unsuccessful!\n", op.c_str());
    exit(1);
  }

}
