#pragma once

#include <stdint.h>

struct AtomicInt {
  alignas(8) long n;
};

#ifdef _WIN32
#include <windows.h>

struct Mutex {
  SRWLOCK srwlock;
};

struct Cond {
  CONDITION_VARIABLE cv;
};

struct RWLock {
  SRWLOCK srwlock;
};

#else
#include <pthread.h>

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

int atomic_int_load(AtomicInt *a);
void atomic_int_store(AtomicInt *a, int val);
int atomic_int_add(AtomicInt *a, int val);
bool atomic_int_cas(AtomicInt *a, int *expect, int val);

void *atomic_ptr_load(void **p);
void atomic_ptr_store(void **p, void *val);
bool atomic_ptr_cas(void **p, void **expect, void *val);

Mutex mutex_make();
void mutex_trash(Mutex *mtx);
void mutex_lock(Mutex *mtx);
void mutex_unlock(Mutex *mtx);
bool mutex_try_lock(Mutex *mtx);

Cond cond_make();
void cond_trash(Cond *cv);
void cond_signal(Cond *cv);
void cond_broadcast(Cond *cv);
void cond_wait(Cond *cv, Mutex *mtx);

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
