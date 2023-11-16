#include "sync.h"
#include <pthread.h>

#ifdef IS_LINUX

Mutex mutex_make() {
  Mutex mtx = {};
  pthread_mutex_init(&mtx.pt, nullptr);
  return mtx;
}

void mutex_trash(Mutex *mtx) { pthread_mutex_destroy(&mtx->pt); }
void mutex_lock(Mutex *mtx) { pthread_mutex_lock(&mtx->pt); }
void mutex_unlock(Mutex *mtx) { pthread_mutex_unlock(&mtx->pt); }

bool mutex_try_lock(Mutex *mtx) {
  i32 err = pthread_mutex_trylock(&mtx->pt);
  return err == 0;
}

Cond cond_make() {
  Cond cv = {};
  pthread_cond_init(&cv.pt, nullptr);
  return cv;
}

void cond_trash(Cond *cv) { pthread_cond_destroy(&cv->pt); }
void cond_signal(Cond *cv) { pthread_cond_signal(&cv->pt); }
void cond_broadcast(Cond *cv) { pthread_cond_broadcast(&cv->pt); }
void cond_wait(Cond *cv, Mutex *mtx) { pthread_cond_wait(&cv->pt, &mtx->pt); }

RWLock rw_make() {
  RWLock rw = {};
  pthread_rwlock_init(&rw.pt, nullptr);
  return rw;
}

void rw_trash(RWLock *rw) { pthread_rwlock_destroy(&rw->pt); }
void rw_shared_lock(RWLock *rw) { pthread_rwlock_rdlock(&rw->pt); }
void rw_shared_unlock(RWLock *rw) { pthread_rwlock_unlock(&rw->pt); }
void rw_unique_lock(RWLock *rw) { pthread_rwlock_wrlock(&rw->pt); }
void rw_unique_unlock(RWLock *rw) { pthread_rwlock_unlock(&rw->pt); }

Thread thread_create(ThreadStart fn, void *udata) {
  Thread t = {};
  pthread_create(&t.pt, nullptr, (void *(*)(void *))fn, udata);
  return t;
}

void thread_join(Thread t) { pthread_join(t.pt, nullptr); }
u64 this_thread_id() { return pthread_self(); }

#endif // IS_LINUX
