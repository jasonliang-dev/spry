#pragma once

#include <stdint.h>

#ifdef _WIN32
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

using Thread = uintptr_t;
typedef int(ThreadStart)(void *);
Thread *thread_make(ThreadStart fn, void *udata);
void thread_join(Thread *t);
uint64_t this_thread_id();

#ifdef _WIN32

struct Mutex {
  SRWLOCK srwlock;
};

#else

struct Mutex {
  pthread_mutex_t pt;
};

struct Cond {
  pthread_cond_t pt;
};

struct RWLock {
  pthread_rwlock_t pt;
};

#endif
