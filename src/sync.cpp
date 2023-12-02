#include "sync.h"
#include "prelude.h"

#ifndef IS_WIN32
#include <errno.h>
#endif

#ifdef IS_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#endif

#ifdef IS_WIN32

int AtomicInt::load() { return _InterlockedCompareExchange(&n, 0, 0); }

void AtomicInt::store(int val) { _InterlockedExchange(&n, val); }

int AtomicInt::add(int val) { return _InterlockedExchangeAdd(&n, val); }

bool AtomicInt::cas(int *expect, int val) {
  *expect = _InterlockedCompareExchange(&n, val, *expect);
  return *expect == val;
}

void *AtomicPtr::load() {
  return _InterlockedCompareExchangePointer(&p, nullptr, nullptr);
}

void AtomicPtr::store(void *val) { _InterlockedExchangePointer(&p, val); }

bool AtomicPtr::cas(void **expect, void *val) {
  *expect = _InterlockedCompareExchangePointer(&p, val, *expect);
  return *expect == val;
}

Mutex::Mutex() : srwlock() {}
Mutex::~Mutex() {}
void Mutex::lock() { AcquireSRWLockExclusive(&srwlock); }
void Mutex::unlock() { ReleaseSRWLockExclusive(&srwlock); }

bool Mutex::try_lock() {
  BOOLEAN ok = TryAcquireSRWLockExclusive(&srwlock);
  return ok != 0;
}

Cond::Cond() { InitializeConditionVariable(&cv); }
Cond::~Cond() {}
void Cond::signal() { WakeConditionVariable(&cv); }
void Cond::broadcast() { WakeAllConditionVariable(&cv); }

void Cond::wait(Mutex *mtx) {
  SleepConditionVariableSRW(&cv, &mtx->srwlock, INFINITE, 0);
}

bool Cond::timed_wait(Mutex *mtx, uint32_t ms) {
  return SleepConditionVariableSRW(&cv, &mtx->srwlock, ms, 0);
}

RWLock::RWLock() : srwlock() {}
RWLock::~RWLock() {}
void RWLock::shared_lock() { AcquireSRWLockShared(&srwlock); }
void RWLock::shared_unlock() { ReleaseSRWLockShared(&srwlock); }
void RWLock::unique_lock() { AcquireSRWLockExclusive(&srwlock); }
void RWLock::unique_unlock() { ReleaseSRWLockExclusive(&srwlock); }

Sema::Sema(int n) { handle = CreateSemaphoreA(nullptr, n, LONG_MAX, nullptr); }
Sema::~Sema() { CloseHandle(handle); }
void Sema::post(int n) { ReleaseSemaphore(handle, n, nullptr); }
void Sema::wait() { WaitForSingleObjectEx(handle, INFINITE, false); }

void Thread::make(ThreadProc fn, void *udata) {
  DWORD id = 0;
  HANDLE handle =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fn, udata, 0, &id);
  ptr = (void *)handle;
}

void Thread::join() {
  WaitForSingleObject((HANDLE)ptr, INFINITE);
  CloseHandle((HANDLE)ptr);
}

uint64_t this_thread_id() { return GetCurrentThreadId(); }

#else

static struct timespec ms_from_now(u32 ms) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  unsigned long long tally = ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
  tally += ms;

  ts.tv_sec = tally / 1000LL;
  ts.tv_nsec = (tally % 1000LL) * 1000000LL;

  return ts;
}

int AtomicInt::load() { return __atomic_load_n(&n, __ATOMIC_SEQ_CST); }

void AtomicInt::store(int val) { __atomic_store_n(&n, val, __ATOMIC_SEQ_CST); }

int AtomicInt::add(int val) {
  return __atomic_fetch_add(&n, val, __ATOMIC_SEQ_CST);
}

bool AtomicInt::cas(int *expect, int val) {
  return __atomic_compare_exchange_n(&n, expect, val, true, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
}

void *AtomicPtr::load() { return __atomic_load_n(&p, __ATOMIC_SEQ_CST); }

void AtomicPtr::store(void *val) {
  __atomic_store_n(&p, val, __ATOMIC_SEQ_CST);
}

bool AtomicPtr::cas(void **expect, void *val) {
  return __atomic_compare_exchange_n(&p, expect, val, true, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
}

Mutex::Mutex() { pthread_mutex_init(&pt, nullptr); }
Mutex::~Mutex() { pthread_mutex_destroy(&pt); }
void Mutex::lock() { pthread_mutex_lock(&pt); }
void Mutex::unlock() { pthread_mutex_unlock(&pt); }

bool Mutex::try_lock() {
  int res = pthread_mutex_trylock(&pt);
  return res == 0;
}

Cond::Cond() { pthread_cond_init(&pt, nullptr); }
Cond::~Cond() { pthread_cond_destroy(&pt); }
void Cond::signal() { pthread_cond_signal(&pt); }
void Cond::broadcast() { pthread_cond_broadcast(&pt); }
void Cond::wait(Mutex *mtx) { pthread_cond_wait(&pt, &mtx->pt); }

bool Cond::timed_wait(Mutex *mtx, uint32_t ms) {
  struct timespec ts = ms_from_now(ms);
  int res = pthread_cond_timedwait(&pt, &mtx->pt, &ts);
  return res == 0;
}

RWLock::RWLock() { pthread_rwlock_init(&pt, nullptr); }
RWLock::~RWLock() { pthread_rwlock_destroy(&pt); }
void RWLock::shared_lock() { pthread_rwlock_rdlock(&pt); }
void RWLock::shared_unlock() { pthread_rwlock_unlock(&pt); }
void RWLock::unique_lock() { pthread_rwlock_wrlock(&pt); }
void RWLock::unique_unlock() { pthread_rwlock_unlock(&pt); }

Sema::Sema(int n) {
  sem = (sem_t *)mem_alloc(sizeof(sem_t));
  sem_init(sem, 0, n);
}

Sema::~Sema() {
  sem_destroy(sem);
  mem_free(sem);
}

void Sema::post(, int n) {
  for (int i = 0; i < n; i++) {
    sem_post(sem);
  }
}

void Sema::wait() { sem_wait(sem); }

Thread::make(ThreadProc fn, void *udata) {
  pthread_t pt = {};
  pthread_create(&pt, nullptr, (void *(*)(void *))fn, udata);
  ptr = (void *)pt;
}

void Thread::join() { pthread_join((pthread_t)ptr, nullptr); }

#endif

#ifdef IS_LINUX

uint64_t this_thread_id() {
  thread_local uint64_t s_tid = syscall(SYS_gettid);
  return s_tid;
}

#endif // IS_LINUX

#ifdef IS_HTML5

uint64_t this_thread_id() { return 0; }

#endif // IS_HTML5
