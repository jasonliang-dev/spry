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

int atomic_int_load(AtomicInt *a) {
  return _InterlockedCompareExchange(&a->n, 0, 0);
}

void atomic_int_store(AtomicInt *a, int val) {
  _InterlockedExchange(&a->n, val);
}

int atomic_int_add(AtomicInt *a, int val) {
  return _InterlockedExchangeAdd(&a->n, val);
}

bool atomic_int_cas(AtomicInt *a, int *expect, int val) {
  *expect = _InterlockedCompareExchange(&a->n, val, *expect);
  return *expect == val;
}

void *atomic_ptr_load(void **p) {
  return _InterlockedCompareExchangePointer(p, nullptr, nullptr);
}

void atomic_ptr_store(void **p, void *val) {
  _InterlockedExchangePointer(p, val);
}

bool atomic_ptr_cas(void **p, void **expect, void *val) {
  *expect = _InterlockedCompareExchangePointer(p, val, *expect);
  return *expect == val;
}

Mutex mutex_make() { return {}; }
void mutex_trash(Mutex *mtx) {}
void mutex_lock(Mutex *mtx) { AcquireSRWLockExclusive(&mtx->srwlock); }
void mutex_unlock(Mutex *mtx) { ReleaseSRWLockExclusive(&mtx->srwlock); }

bool mutex_try_lock(Mutex *mtx) {
  BOOLEAN ok = TryAcquireSRWLockExclusive(&mtx->srwlock);
  return ok != 0;
}

Cond cond_make() {
  Cond cv = {};
  InitializeConditionVariable(&cv.cv);
  return cv;
}

void cond_trash(Cond *cv) {}
void cond_signal(Cond *cv) { WakeConditionVariable(&cv->cv); }
void cond_broadcast(Cond *cv) { WakeAllConditionVariable(&cv->cv); }

void cond_wait(Cond *cv, Mutex *mtx) {
  SleepConditionVariableSRW(&cv->cv, &mtx->srwlock, INFINITE, 0);
}

bool cond_timed_wait(Cond *cv, Mutex *mtx, uint32_t ms) {
  return SleepConditionVariableSRW(&cv->cv, &mtx->srwlock, ms, 0);
}

RWLock rw_make() { return {}; }
void rw_trash(RWLock *rw) {}
void rw_shared_lock(RWLock *rw) { AcquireSRWLockShared(&rw->srwlock); }
void rw_shared_unlock(RWLock *rw) { ReleaseSRWLockShared(&rw->srwlock); }
void rw_unique_lock(RWLock *rw) { AcquireSRWLockExclusive(&rw->srwlock); }
void rw_unique_unlock(RWLock *rw) { ReleaseSRWLockExclusive(&rw->srwlock); }

Sema sema_make(int n) {
  Sema s = {};
  s.handle = CreateSemaphoreA(nullptr, n, LONG_MAX, nullptr);
  return s;
}

void sema_trash(Sema *s) { CloseHandle(s->handle); }
void sema_post(Sema *s, int n) { ReleaseSemaphore(s->handle, n, nullptr); }
void sema_wait(Sema *s) { WaitForSingleObjectEx(s->handle, INFINITE, false); }

Thread *thread_make(ThreadStart fn, void *udata) {
  DWORD id = 0;
  HANDLE handle =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fn, udata, 0, &id);
  return (Thread *)handle;
}

void thread_join(Thread *t) {
  WaitForSingleObject((HANDLE)t, INFINITE);
  CloseHandle((HANDLE)t);
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

int atomic_int_load(AtomicInt *a) {
  return __atomic_load_n(a, __ATOMIC_SEQ_CST);
}

void atomic_int_store(AtomicInt *a, int val) {
  __atomic_store_n(a, val, __ATOMIC_SEQ_CST);
}

int atomic_int_add(AtomicInt *a, int val) {
  return __atomic_fetch_add(a, val, __ATOMIC_SEQ_CST);
}

bool atomic_int_cas(AtomicInt *a, int *expect, int val) {
  return __atomic_compare_exchange_n(a, expect, val, true, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
}

void *atomic_ptr_load(void **p) { return __atomic_load_n(p, __ATOMIC_SEQ_CST); }

void atomic_ptr_store(void **p, void *val) {
  __atomic_store_n(p, val, __ATOMIC_SEQ_CST);
}

bool atomic_ptr_cas(void **p, void **expect, void *val) {
  return __atomic_compare_exchange_n(p, expect, val, true, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
}

Mutex mutex_make() {
  Mutex mtx = {};
  pthread_mutex_init(&mtx.pt, nullptr);
  return mtx;
}

void mutex_trash(Mutex *mtx) { pthread_mutex_destroy(&mtx->pt); }
void mutex_lock(Mutex *mtx) { pthread_mutex_lock(&mtx->pt); }
void mutex_unlock(Mutex *mtx) { pthread_mutex_unlock(&mtx->pt); }

bool mutex_try_lock(Mutex *mtx) {
  int res = pthread_mutex_trylock(&mtx->pt);
  return res == 0;
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

bool cond_timed_wait(Cond *cv, Mutex *mtx, uint32_t ms) {
  struct timespec ts = ms_from_now(ms);
  int res = pthread_cond_timedwait(&cv->pt, &mtx->pt, &ts);
  return res == 0;
}

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

Sema sema_make(int n) {
  Sema s = {};
  s.sem = (sem_t *)mem_alloc(sizeof(sem_t));
  sem_init(s.sem, 0, n);
  return s;
}

void sema_trash(Sema *s) {
  sem_destroy(s->sem);
  mem_free(s->sem);
}

void sema_post(Sema *s, int n) {
  for (int i = 0; i < n; i++) {
    sem_post(s->sem);
  }
}

void sema_wait(Sema *s) { sem_wait(s->sem); }

Thread *thread_make(ThreadStart fn, void *udata) {
  pthread_t pt = {};
  pthread_create(&pt, nullptr, (void *(*)(void *))fn, udata);
  return (Thread *)pt;
}

void thread_join(Thread *t) { pthread_join((pthread_t)t, nullptr); }

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