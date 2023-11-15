#include "profile.h"
#include "deps/cute_sync.h"
#include "deps/sokol_time.h"
#include "os.h"
#include "queue.h"
#include "strings.h"

struct Profile {
  cute_mutex_t mtx;
  cute_cv_t cond;
  Queue<TraceEvent> events;

  cute_thread_t *recv_thread;
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
    cute_lock(&g_profile.mtx);
    while (g_profile.events.len == 0) {
      cute_cv_wait(&g_profile.cond, &g_profile.mtx);
    }

    TraceEvent e = {};
    queue_pop(&g_profile.events, &e);
    cute_unlock(&g_profile.mtx);

    if (e.name == nullptr) {
      return 0;
    }

    fprintf(f,
            R"({"name":"%s","cat":"%s","ph":"%c","ts":%.3f,"pid":0,"tid":%d},)"
            "\n",
            e.name, e.cat, e.ph, stm_us(e.ts), e.tid);
  }
}

void profile_send(TraceEvent e) {
  cute_lock(&g_profile.mtx);
  queue_push(&g_profile.events, e);
  cute_unlock(&g_profile.mtx);

  cute_cv_wake_one(&g_profile.cond);
}

void profile_setup() {
  os_high_timer_resolution();
  stm_setup();

  g_profile = {};

  g_profile.mtx = cute_mutex_create();
  g_profile.cond = cute_cv_create();
  g_profile.recv_thread =
      cute_thread_create(profile_recv_thread, "profile", nullptr);

  queue_reserve(&g_profile.events, 256);
}

void profile_shutdown() {
  profile_send({});
  cute_thread_wait(g_profile.recv_thread);

  cute_cv_destroy(&g_profile.cond);
  cute_mutex_destroy(&g_profile.mtx);

  queue_trash(&g_profile.events);
}

Instrument::Instrument(const char *cat, const char *name)
    : cat(cat), name(name), tid(os_thread_id()) {
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