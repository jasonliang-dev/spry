#pragma once

void profile_setup();
void profile_shutdown();

#ifdef DEBUG
#ifndef USE_PROFILER
#define USE_PROFILER
#endif
#endif

#ifdef USE_PROFILER
#include "deps/cute_sync.h"
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

struct Instrument {
  const char *cat;
  const char *name;
  i32 tid;

  Instrument(const char *cat, const char *name);
  ~Instrument();
};

#define PROFILE_FUNC()                                                         \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("function", __func__);

#define PROFILE_BLOCK(name)                                                    \
  auto JOIN_2(_profile_, __COUNTER__) = Instrument("block", name);

#endif // USE_PROFILER

#ifndef USE_PROFILER
#define PROFILE_FUNC()
#define PROFILE_BLOCK(name)
#endif
