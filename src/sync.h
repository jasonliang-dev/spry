#pragma once

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>

struct AtomicInt {
  alignas(8) long n;
};

struct Mutex {
  SRWLOCK srwlock;
};

struct Cond {
  CONDITION_VARIABLE cv;
};

struct RWLock {
  SRWLOCK srwlock;
};

struct Sema {
  HANDLE handle;
};

#else
#include <pthread.h>
#include <semaphore.h>

using AtomicInt = int;

struct Mutex {
  pthread_mutex_t pt;
};

struct Cond {
  pthread_cond_t pt;
};

struct RWLock {
  pthread_rwlock_t pt;
};

struct Sema {
  sem_t *sem;
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
bool cond_timed_wait(Cond *cv, Mutex *mtx, uint32_t ms);

RWLock rw_make();
void rw_trash(RWLock *rw);
void rw_shared_lock(RWLock *rw);
void rw_shared_unlock(RWLock *rw);
void rw_unique_lock(RWLock *rw);
void rw_unique_unlock(RWLock *rw);

Sema sema_make(int n);
void sema_trash(Sema *s);
void sema_post(Sema *s, int n);
void sema_wait(Sema *s);

using Thread = uintptr_t;
typedef int(ThreadStart)(void *);

Thread *thread_make(ThreadStart fn, void *udata);
void thread_join(Thread *t);
uint64_t this_thread_id();
