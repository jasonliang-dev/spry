#include "profile.h"
#include "chan.h"

#ifndef USE_PROFILER
void profile_setup() {}
void profile_shutdown() {}
#endif

#ifdef USE_PROFILER

#include "deps/sokol_time.h"
#include "os.h"
#include "strings.h"
#include "sync.h"

struct Profile {
  Chan<TraceEvent> events;
  Thread *recv_thread;
};

static Profile g_profile;

static i32 profile_recv_thread(void *) {
  StringBuilder sb = string_builder_make();
  defer(string_builder_trash(&sb));

  string_builder_swap_filename(&sb, os_program_path(), "profile.json");

  FILE *f = fopen(sb.data, "w");
  defer(fclose(f));

  fputs("[", f);
  while (true) {
    TraceEvent e = chan_recv(&g_profile.events);
    if (e.name == nullptr) {
      return 0;
    }

    fprintf(
        f,
        R"({"name":"%s","cat":"%s","ph":"%c","ts":%.3f,"pid":0,"tid":%hu},)"
        "\n",
        e.name, e.cat, e.ph, stm_us(e.ts), e.tid);
  }
}

void profile_setup() {
  g_profile = {};

  chan_make(&g_profile.events);
  g_profile.recv_thread = thread_make(profile_recv_thread, nullptr);

  chan_reserve(&g_profile.events, 256);
}

void profile_shutdown() {
  chan_send(&g_profile.events, {});
  thread_join(g_profile.recv_thread);

  chan_trash(&g_profile.events);
}

Instrument::Instrument(const char *cat, const char *name)
    : cat(cat), name(name), tid(this_thread_id()) {
  TraceEvent e = {};
  e.cat = cat;
  e.name = name;
  e.ph = 'B';
  e.ts = stm_now();
  e.tid = tid;

  chan_send(&g_profile.events, e);
}

Instrument::~Instrument() {
  TraceEvent e = {};
  e.cat = cat;
  e.name = name;
  e.ph = 'E';
  e.ts = stm_now();
  e.tid = tid;

  chan_send(&g_profile.events, e);
}

#endif // USE_PROFILER