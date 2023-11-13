#pragma once

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
#include "deps/sokol_time.h"
#include "os.h"
#include "queue.h"
#include "strings.h"

struct TraceEvent {
  const char *cat;
  const char *name;
  u64 start;
  u64 end;
  i32 tid;
};

struct Profile {
  cute_semaphore_t send;
  cute_mutex_t mtx;
  cute_thread_t *recv_thread;

  Queue<TraceEvent> events;
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

    TraceEvent e = {};
    cute_lock(&g_profile.mtx);
    queue_pop(&g_profile.events, &e);
    cute_unlock(&g_profile.mtx);

    fprintf(
        f,
        R"({"name":"%s","cat":"%s","ph":"X","ts":%.3f,"dur":%.3f,"pid":0,"tid":%d},)"
        "\n",
        e.name, e.cat, stm_us(e.start), stm_us(stm_diff(e.end, e.start)),
        e.tid);
  }
}

inline void profile_setup() {
  g_profile = {};

  g_profile.send = cute_semaphore_create(0);
  g_profile.mtx = cute_mutex_create();
  g_profile.recv_thread =
      cute_thread_create(profile_recv_thread, "profile", nullptr);
}

inline void profile_shutdown() {
  cute_semaphore_destroy(&g_profile.send);
  cute_thread_wait(g_profile.recv_thread);

  cute_mutex_destroy(&g_profile.mtx);

  queue_trash(&g_profile.events);
}

struct Instrument {
  const char *cat;
  const char *name;
  u64 start;

  Instrument(const char *cat, const char *name)
      : cat(cat), name(name), start(stm_now()) {}

  ~Instrument() {
    i32 tid = os_thread_id();
    u64 end = stm_now();

    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.start = start;
    e.end = end;
    e.tid = tid;

    cute_lock(&g_profile.mtx);
    queue_push(&g_profile.events, e);
    cute_unlock(&g_profile.mtx);

    cute_semaphore_post(&g_profile.send);
  }
};

#define PROFILE_FUNC()                                                         \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("function", __func__);

#define PROFILE_BLOCK(name)                                                    \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("block", name);

#endif // USE_PROFILER
