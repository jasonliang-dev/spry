#pragma once

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

struct Mutex {
#ifdef _WIN32
  SRWLOCK srwlock;
#else
  pthread_mutex_t pt;
#endif

  void make();
  void trash();
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

  void make();
  void trash();
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

  void make();
  void trash();
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

  void make(int n = 0);
  void trash();
  void post(int n = 1);
  void wait();
};

typedef void (*ThreadProc)(void *);

struct Thread {
  void *ptr = nullptr;

  void make(ThreadProc fn, void *udata);
  void join();
};

struct LockGuard {
  Mutex *mtx;

  LockGuard(Mutex *mtx) : mtx(mtx) { mtx->lock(); };
  ~LockGuard() { mtx->unlock(); };
  LockGuard(LockGuard &&) = delete;
  LockGuard &operator=(LockGuard &&) = delete;

  operator bool() { return true; }
};

uint64_t this_thread_id();
