#ifndef __SERVER_SCOPED_LOCK__
#define __SERVER_SCOPED_LOCK__

#include <pthread.h>
#include <lock_protocol.h>
#include <lock_client.h>
struct ServerScopedLock {
	private:
        lock_client *lc_;
        lock_protocol::lockid_t id_;
	public:
		ServerScopedLock(lock_client *lc, lock_protocol::lockid_t id) {
            lc_ = lc;
            id_ = id;
            printf("        acquired lock: %016llu \n", id_);
			lc_->acquire(id_);
		}
		~ServerScopedLock() {
            printf("        released lock: %016llu \n", id_);
			lc_->release(id_);
		}
};
#endif  /*__SERVER_SCOPED_LOCK__*/
