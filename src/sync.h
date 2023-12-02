#pragma once

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

struct AtomicInt {
#ifdef _WIN32
  alignas(8) long n;
#else
  int n;
#endif

  int load();
  void store(int val);
  int add(int val);
  bool cas(int *expect, int val);
};

struct AtomicPtr {
  void *p;

  void *load();
  void store(void *val);
  bool cas(void **expect, void *val);
};

struct Mutex {
#ifdef _WIN32
  SRWLOCK srwlock;
#else
  pthread_mutex_t pt;
#endif

  Mutex();
  ~Mutex();
  Mutex(Mutex &&rhs) = delete;
  Mutex &operator=(Mutex &&rhs) = delete;

  void lock();
  void unlock();
  bool try_lock();
};

struct Cond {
#ifdef _WIN32
  CONDITION_VARIABLE cv;
#else
  pthread_cond_t pt;
#endif

  Cond();
  ~Cond();
  Cond(Cond &&rhs) = delete;
  Cond &operator=(Cond &&rhs) = delete;

  void signal();
  void broadcast();
  void wait(Mutex *mtx);
  bool timed_wait(Mutex *mtx, uint32_t ms);
};

struct RWLock {
#if _WIN32
  SRWLOCK srwlock;
#else
  pthread_rwlock_t pt;
#endif

  RWLock();
  ~RWLock();
  RWLock(RWLock &&rhs) = delete;
  RWLock &operator=(RWLock &&rhs) = delete;

  void shared_lock();
  void shared_unlock();
  void unique_lock();
  void unique_unlock();
};

struct Sema {
#ifdef _WIN32
  HANDLE handle;
#else
  sem_t *sem;
#endif

  Sema(int n = 1);
  ~Sema();
  Sema(Sema &&rhs) = delete;
  Sema &operator=(Sema &&rhs) = delete;

  void post(int n = 1);
  void wait();
};

typedef void (*ThreadProc)(void *);

struct Thread {
  void *ptr;

  void make(ThreadProc fn, void *udata);
  void join();
};

uint64_t this_thread_id();
