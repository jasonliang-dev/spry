#pragma once

#ifdef RELEASE
#define PROFILE_FUNC()
#define PROFILE_BLOCK(name)
#endif

#ifdef DEBUG
#include "array.h"
#include "deps/sokol_time.h"

struct TraceEvent {
  const char *cat;
  const char *name;
  u64 start;
  u64 end;
};

struct Profile {
  i32 process_id;
  i32 thread_id;
  Array<TraceEvent> events;
};

extern Profile g_profile;

struct Instrument {
  const char *cat;
  const char *name;
  u64 start;

  Instrument(const char *cat, const char *name)
      : cat(cat), name(name), start(stm_now()) {}

  ~Instrument() {
    u64 end = stm_now();

    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.start = start;
    e.end = end;

    array_push(&g_profile.events, e);
  }
};

#define PROFILE_FUNC()                                                         \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("function", __func__);

#define PROFILE_BLOCK(name)                                                    \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("block", name);

#endif
