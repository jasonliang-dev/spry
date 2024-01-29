#include "profile.h"
#include "queue.h"

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
  Queue<TraceEvent> events;
  Thread recv_thread;
};

static Profile g_profile = {};

static void profile_recv_thread(void *) {
  StringBuilder sb = {};
  sb.swap_filename(os_program_path(), "profile.json");

  FILE *f = fopen(sb.data, "w");
  sb.trash();

  defer(fclose(f));

  fputs("[", f);
  while (true) {
    TraceEvent e = g_profile.events.demand();
    if (e.name == nullptr) {
      return;
    }

    fprintf(f,
            R"({"name":"%s","cat":"%s","ph":"%c","ts":%.3f,"pid":0,"tid":%hu},)"
            "\n",
            e.name, e.cat, e.ph, stm_us(e.ts), e.tid);
  }
}

void profile_setup() {
  g_profile.events.make();
  g_profile.events.reserve(256);
  g_profile.recv_thread.make(profile_recv_thread, nullptr);
}

void profile_shutdown() {
  g_profile.events.enqueue({});
  g_profile.recv_thread.join();
  g_profile.events.trash();
}

Instrument::Instrument(const char *cat, const char *name)
    : cat(cat), name(name), tid(this_thread_id()) {
  TraceEvent e = {};
  e.cat = cat;
  e.name = name;
  e.ph = 'B';
  e.ts = stm_now();
  e.tid = tid;

  g_profile.events.enqueue(e);
}

Instrument::~Instrument() {
  TraceEvent e = {};
  e.cat = cat;
  e.name = name;
  e.ph = 'E';
  e.ts = stm_now();
  e.tid = tid;

  g_profile.events.enqueue(e);
}

#endif // USE_PROFILER