#pragma once

#include "os.h"

#ifdef IS_WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

struct Mutex;
Mutex mutex_make();
void mutex_trash(Mutex *mtx);
void mutex_lock(Mutex *mtx);
void mutex_unlock(Mutex *mtx);
bool mutex_try_lock(Mutex *mtx);

struct Cond;
Cond cond_make();
void cond_trash(Cond *cv);
void cond_signal(Cond *cv);
void cond_broadcast(Cond *cv);
void cond_wait(Cond *cv, Mutex *mtx);

struct RWLock;
RWLock rw_make();
void rw_trash(RWLock *rw);
void rw_shared_lock(RWLock *rw);
void rw_shared_unlock(RWLock *rw);
void rw_unique_lock(RWLock *rw);
void rw_unique_unlock(RWLock *rw);

struct Thread;
typedef i32(ThreadStart)(void *);
Thread thread_create(ThreadStart fn, void *udata);
void thread_join(Thread t);
u64 this_thread_id();

#ifdef IS_WIN32

struct Mutex {
  SRWLOCK srwlock;
};

#endif // IS_WIN32

#ifdef IS_LINUX

struct Mutex {
  pthread_mutex_t pt;
};

struct Cond {
  pthread_cond_t pt;
};

struct RWLock {
  pthread_rwlock_t pt;
};

struct Thread {
  pthread_t pt;
};

#endif // IS_LINUX
