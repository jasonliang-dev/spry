#include "profile.h"

#ifndef USE_PROFILER
void profile_setup() {}
void profile_shutdown() {}
#endif

#ifdef USE_PROFILER

#include "deps/sokol_time.h"
#include "os.h"
#include "queue.h"
#include "strings.h"
#include "sync.h"

struct Profile {
  Mutex mtx;
  Cond cv;
  Queue<TraceEvent> events;

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
    mutex_lock(&g_profile.mtx);
    while (g_profile.events.len == 0) {
      cond_wait(&g_profile.cv, &g_profile.mtx);
    }

    TraceEvent e = {};
    queue_pop(&g_profile.events, &e);
    mutex_unlock(&g_profile.mtx);

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

static void profile_send(TraceEvent e) {
  mutex_lock(&g_profile.mtx);
  queue_push(&g_profile.events, e);
  mutex_unlock(&g_profile.mtx);

  cond_signal(&g_profile.cv);
}

void profile_setup() {
  g_profile = {};

  g_profile.mtx = mutex_make();
  g_profile.cv = cond_make();
  g_profile.recv_thread = thread_make(profile_recv_thread, nullptr);

  queue_reserve(&g_profile.events, 256);
}

void profile_shutdown() {
  profile_send({});
  thread_join(g_profile.recv_thread);

  cond_trash(&g_profile.cv);
  mutex_trash(&g_profile.mtx);

  queue_trash(&g_profile.events);
}

Instrument::Instrument(const char *cat, const char *name)
    : cat(cat), name(name), tid(this_thread_id()) {
  TraceEvent e = {};
  e.cat = cat;
  e.name = name;
  e.ph = 'B';
  e.ts = stm_now();
  e.tid = tid;

  profile_send(e);
}

Instrument::~Instrument() {
  TraceEvent e = {};
  e.cat = cat;
  e.name = name;
  e.ph = 'E';
  e.ts = stm_now();
  e.tid = tid;

  profile_send(e);
}

#endif // USE_PROFILER