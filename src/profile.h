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
  char ph;
  u64 ts;
  i32 tid;
};

struct Profile {
  cute_semaphore_t send;
  cute_thread_t *recv_thread;

  cute_mutex_t mtx;
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
    {
      cute_lock(&g_profile.mtx);
      queue_pop(&g_profile.events, &e);
      cute_unlock(&g_profile.mtx);
    }

    fprintf(
        f,
        R"({"name":"%s","cat":"%s","ph":"%c","ts":%.3f,"pid":0,"tid":%d},)"
        "\n",
        e.name, e.cat, e.ph, stm_us(e.ts), e.tid);
  }
}

inline void profile_setup() {
  g_profile = {};

  g_profile.send = cute_semaphore_create(0);
  g_profile.mtx = cute_mutex_create();
  g_profile.recv_thread =
      cute_thread_create(profile_recv_thread, "profile", nullptr);

  queue_reserve(&g_profile.events, 256);
}

inline void profile_shutdown() {
  cute_semaphore_destroy(&g_profile.send);
  cute_thread_wait(g_profile.recv_thread);

  cute_mutex_destroy(&g_profile.mtx);

  queue_trash(&g_profile.events);
}

inline void profile_send(TraceEvent e) {
  cute_lock(&g_profile.mtx);
  queue_push(&g_profile.events, e);
  cute_unlock(&g_profile.mtx);

  cute_semaphore_post(&g_profile.send);
}

struct Instrument {
  const char *cat;
  const char *name;
  i32 tid;

  Instrument(const char *cat, const char *name)
      : cat(cat), name(name), tid(os_thread_id()) {
    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.ph = 'B';
    e.ts = stm_now();
    e.tid = tid;

    profile_send(e);
  }

  ~Instrument() {
    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.ph = 'E';
    e.ts = stm_now();
    e.tid = tid;

    profile_send(e);
  }
};

#define PROFILE_FUNC()                                                         \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("function", __func__);

#define PROFILE_BLOCK(name)                                                    \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("block", name);

#endif // USE_PROFILER
