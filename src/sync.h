#pragma once

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

struct Mutex;
struct LockGuard {
  Mutex *mtx;

  LockGuard(Mutex *mtx);
  ~LockGuard();
  LockGuard(LockGuard &&) = delete;
  LockGuard &operator=(LockGuard &&) = delete;

  operator bool() { return true; }
};

struct Mutex {
#ifdef _WIN32
  SRWLOCK srwlock;
#else
  pthread_mutex_t pt;
#endif

  Mutex();
  ~Mutex();
  Mutex(Mutex &&) = delete;
  Mutex &operator=(Mutex &&) = delete;

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
  Cond(Cond &&) = delete;
  Cond &operator=(Cond &&) = delete;

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
  RWLock(RWLock &&) = delete;
  RWLock &operator=(RWLock &&) = delete;

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

  Sema(int n = 0);
  ~Sema();
  Sema(Sema &&) = delete;
  Sema &operator=(Sema &&) = delete;

  void post(int n = 1);
  void wait();
};

typedef void (*ThreadProc)(void *);

struct Thread {
  void *ptr = nullptr;

  void make(ThreadProc fn, void *udata);
  void join();
};

uint64_t this_thread_id();
