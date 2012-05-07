// this is the lock server
// the lock client has a similar interface

#ifndef lock_server_h
#define lock_server_h

#include <string>
#include <map>
#include "lock_protocol.h"
#include "lock_client.h"
#include "rpc.h"

class lock_entry {
  public:
    lock_entry() {
      locked = false;
      pthread_cond_init(&condition, NULL);
    };

    bool is_locked() {
      return locked;
    };

    void set_locked(bool b) {
      locked = b;
    };

    pthread_cond_t get_cond() {
      return condition;
    };

    void wait(pthread_mutex_t *mutex) {
      pthread_cond_wait(&condition, mutex);
    };

    void signal() {
      pthread_cond_signal(&condition);
    };

  private:
    bool locked;
    pthread_cond_t condition;

};

class lock_server {
  private:
    std::map<lock_protocol::lockid_t, lock_entry> locks_;
    pthread_mutex_t mutex_;
 
  protected:
    int nacquire;
 
  public:
    lock_server();
    ~lock_server() {};
    lock_protocol::status stat(int clt, lock_protocol::lockid_t lid, int &);
    lock_protocol::status acquire(int clt, lock_protocol::lockid_t lid, int &);
    lock_protocol::status release(int clt, lock_protocol::lockid_t lid, int &);
};

#endif 
