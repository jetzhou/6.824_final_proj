// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
  pthread_mutex_init(&mutex_, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
/*
  // these don't matter so just init here
  lock_protocol::status ret = lock_protocol::OK;
  r = nacquire;
  
  // if lid has never been seen before or the lid is FREE
  // set lid to LOCKED and return
  pthread_mutex_lock(&mutex);
  printf("acquire request from clt %d\n", clt);
  
  if (locks.count(lid) == 0 || locks[lid] == FREE)  {
    locks[lid] = LOCKED;
    printf("    granted lock %d to clt %d, new or free\n", lid, clt);
    pthread_mutex_unlock(&mutex);
    return ret;
  }

  // otherwise we wait until it's FREE and then return
  while (locks[lid] == LOCKED) {
    pthread_cond_wait(&condition, &mutex);
    if (locks[lid] == FREE) {
      locks[lid] = LOCKED;
      printf("    granted lock %d to clt %d, after waiting\n", lid, clt);
      pthread_mutex_unlock(&mutex);
      return ret;
    }
  }
*/

  // these don't matter so just init here
  lock_protocol::status ret = lock_protocol::OK;
  r = nacquire;

  // if lid has never been seen before
  // initialize lock_entry, set lid to LOCKED, and return
  pthread_mutex_lock(&mutex_);
  
  if (locks_.count(lid) == 0)  {
    lock_entry entry;
    entry.set_locked(true);
    locks_[lid] = entry;
    pthread_mutex_unlock(&mutex_);
    return ret;
  }

  // otherwise we wait until it's free and then return
  while (locks_[lid].is_locked()) {
    locks_[lid].wait(&mutex_);
  }
  
  // the lock is free at this point
  locks_[lid].set_locked(true);
  pthread_mutex_unlock(&mutex_);
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
/*
  pthread_mutex_lock(&mutex);
  locks[lid] = FREE;
  pthread_cond_signal(&condition);
  lock_protocol::status ret = lock_protocol::OK;
  printf("release lock %d request from clt %d\n", lid, clt);
  r = nacquire;
  pthread_mutex_unlock(&mutex);
  return ret;
*/

  // set lock free and signal
  pthread_mutex_lock(&mutex_);
  locks_[lid].set_locked(false);
  locks_[lid].signal();
  r = nacquire;
  pthread_mutex_unlock(&mutex_);
  return lock_protocol::OK;
}

