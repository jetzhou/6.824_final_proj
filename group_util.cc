#include "group_util.h"
#include <string>

group_util::group_util(std::string dst) {
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() != 0) {
    printf("group_util: bind failed\n");
  }
}


int
group_util::groupadd(extent_protocol::groupid_t) {
  return 0;
}


int
group_util::groupdel(extent_protocol::groupid_t) {
  return 0;
}


int
group_util::useradd(extent_protocol::userid_t, extent_protocol::groupid_t) {
  return 0;
}

int
group_util::userdel(extent_protocol::userid_t, extent_protocol::groupid_t) {
  return 0;
}

int
main(int argc, char *argv[]) {

  if (argc != 6) {
    fprintf(stderr, "Usage: grouputil <port-extent-server> <operation> <username> <groupname> <yourid> <youruserkey>\n");
    fprintf(stderr, "Possible operations are: groupadd, groupdel, useradd, userdel\n");
    fprintf(stderr, "Use NA if no group/user name is available\n");
    exit(1);
  }

  group_util util = new groputil(argv[1], argv[3], argv[4], argv[5], argv[6]);

  std::string op = new std::string(argv[2]);

  if (op == "groupadd") {
    util.groupadd(argv[3], argv[4], argv[5], argv[6]);
  } else if (op == "groupdel") {
    util.groupdel(argv[3], argv[4], argv[5], argv[6]);
  } else if (op == "useradd") {
    util.useradd(argv[3], argv[4], argv[5], argv[6]);
  } else if (op == "userdel") {
    util.userdel(argv[3], argv[4], argv[5], argv[6]);
  } else {
    fprintf(stderr, "Usage: grouputil <port-extent-server> <operation> <username> <groupname> <yourid> <youruserkey>\n");
    fprintf(stderr, "Possible operations are: groupadd, groupdel, useradd, userdel\n");
    fprintf(stderr, "Use NA if no group/user name is available\n");
    exit(1);
  }

  exit(0);

}
