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

void Mutex::make() { srwlock = {}; }
void Mutex::trash() {}
void Mutex::lock() { AcquireSRWLockExclusive(&srwlock); }
void Mutex::unlock() { ReleaseSRWLockExclusive(&srwlock); }

bool Mutex::try_lock() {
  BOOLEAN ok = TryAcquireSRWLockExclusive(&srwlock);
  return ok != 0;
}

void Cond::make() { InitializeConditionVariable(&cv); }
void Cond::trash() {}
void Cond::signal() { WakeConditionVariable(&cv); }
void Cond::broadcast() { WakeAllConditionVariable(&cv); }

void Cond::wait(Mutex *mtx) {
  SleepConditionVariableSRW(&cv, &mtx->srwlock, INFINITE, 0);
}

bool Cond::timed_wait(Mutex *mtx, uint32_t ms) {
  return SleepConditionVariableSRW(&cv, &mtx->srwlock, ms, 0);
}

void RWLock::make() { srwlock = {}; }
void RWLock::trash() {}
void RWLock::shared_lock() { AcquireSRWLockShared(&srwlock); }
void RWLock::shared_unlock() { ReleaseSRWLockShared(&srwlock); }
void RWLock::unique_lock() { AcquireSRWLockExclusive(&srwlock); }
void RWLock::unique_unlock() { ReleaseSRWLockExclusive(&srwlock); }

void Sema::make(int n) {
  handle = CreateSemaphoreA(nullptr, n, LONG_MAX, nullptr);
}
void Sema::trash() { CloseHandle(handle); }
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

void Mutex::make() { pthread_mutex_init(&pt, nullptr); }
void Mutex::trash() { pthread_mutex_destroy(&pt); }
void Mutex::lock() { pthread_mutex_lock(&pt); }
void Mutex::unlock() { pthread_mutex_unlock(&pt); }

bool Mutex::try_lock() {
  int res = pthread_mutex_trylock(&pt);
  return res == 0;
}

void Cond::make() { pthread_cond_init(&pt, nullptr); }
void Cond::trash() { pthread_cond_destroy(&pt); }
void Cond::signal() { pthread_cond_signal(&pt); }
void Cond::broadcast() { pthread_cond_broadcast(&pt); }
void Cond::wait(Mutex *mtx) { pthread_cond_wait(&pt, &mtx->pt); }

bool Cond::timed_wait(Mutex *mtx, uint32_t ms) {
  struct timespec ts = ms_from_now(ms);
  int res = pthread_cond_timedwait(&pt, &mtx->pt, &ts);
  return res == 0;
}

void RWLock::make() { pthread_rwlock_init(&pt, nullptr); }
void RWLock::trash() { pthread_rwlock_destroy(&pt); }
void RWLock::shared_lock() { pthread_rwlock_rdlock(&pt); }
void RWLock::shared_unlock() { pthread_rwlock_unlock(&pt); }
void RWLock::unique_lock() { pthread_rwlock_wrlock(&pt); }
void RWLock::unique_unlock() { pthread_rwlock_unlock(&pt); }

void Sema::make(int n) {
  sem = (sem_t *)mem_alloc(sizeof(sem_t));
  sem_init(sem, 0, n);
}

void Sema::trash() {
  sem_destroy(sem);
  mem_free(sem);
}

void Sema::post(int n) {
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
