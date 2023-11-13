#pragma once

#include "strings.h"
#ifdef DEBUG
#ifndef USE_PROFILER
#define USE_PROFILER
#endif
#endif

#ifndef USE_PROFILER
#define PROFILE_FUNC()
#define PROFILE_BLOCK(name)
#endif

#ifdef USE_PROFILER
#include "deps/cute_sync.h"
#include "deps/sokol_time.h"
#include "os.h"

struct TraceEvent {
  const char *cat;
  const char *name;
  u64 start;
  u64 end;
  i32 tid;
};

struct Profile {
  TraceEvent events[256];
  cute_semaphore_t send;
  cute_semaphore_t recv;
  cute_atomic_int_t front;
  cute_atomic_int_t back;
  cute_atomic_int_t len;

  cute_thread_t *recv_thread;
};

extern Profile g_profile;

inline i32 profile_recv_thread(void *) {
  StringBuilder sb = string_builder_make();
  defer(string_builder_trash(&sb));

  string_builder_swap_filename(&sb, os_program_path(), "profile.json");

  FILE *f = fopen(sb.data, "w");
  defer(fclose(f));

  fputs("[", f);
  while (true) {
    bool ok = cute_semaphore_wait(&g_profile.send);
    if (!ok) {
      return 1;
    }

    i32 front = cute_atomic_add(&g_profile.front, 1);
    front &= array_size(Profile::events) - 1;

    TraceEvent e = g_profile.events[front];

    fprintf(
        f,
        R"({"name":"%s","cat":"%s","ph":"X","ts":%.3f,"dur":%.3f,"pid":0,"tid":%d},)"
        "\n",
        e.name, e.cat, stm_us(e.start), stm_us(stm_diff(e.end, e.start)),
        e.tid);

    i32 len = cute_atomic_add(&g_profile.len, -1);
    assert(len != 0);
    if (len == array_size(Profile::events)) {
      cute_semaphore_post(&g_profile.recv);
    }
  }
}

inline void profile_setup() {
  g_profile.send = cute_semaphore_create(0);
  g_profile.recv = cute_semaphore_create(0);
  g_profile.recv_thread =
      cute_thread_create(profile_recv_thread, "profile", nullptr);
}

inline void profile_shutdown() {
  cute_semaphore_destroy(&g_profile.send);
  cute_semaphore_destroy(&g_profile.recv);
  cute_thread_wait(g_profile.recv_thread);
}

inline void profile_send(TraceEvent e) {
  if (cute_atomic_get(&g_profile.len) == array_size(Profile::events)) {
    cute_semaphore_wait(&g_profile.recv);
  }
  cute_atomic_add(&g_profile.len, 1);

  i32 back = cute_atomic_add(&g_profile.back, 1);
  back &= array_size(Profile::events) - 1;

  g_profile.events[back] = e;
  cute_semaphore_post(&g_profile.send);
}

struct Instrument {
  const char *cat;
  const char *name;
  u64 start;

  Instrument(const char *cat, const char *name)
      : cat(cat), name(name), start(stm_now()) {}

  ~Instrument() noexcept {
    i32 tid = os_thread_id();
    u64 end = stm_now();

    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.start = start;
    e.end = end;
    e.tid = tid;

    profile_send(e);
  }
};

#define PROFILE_FUNC()                                                         \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("function", __func__);

#define PROFILE_BLOCK(name)                                                    \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("block", name);

#endif
