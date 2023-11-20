#pragma once

#define SPRY_VERSION "0.7"

#include "array.h"
#include "deps/lua/lua.h"
#include "deps/luaalloc.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "font.h"
#include "sound.h"

struct AppTime {
  u64 last;
  u64 accumulator;
  u64 target_ticks;
  double delta;
};

struct App {
  Mutex frame_mtx;

  LuaAlloc *LA;
  lua_State *L;

  AppTime time;

  AtomicInt hot_reload_enabled;
  AtomicInt reload_interval;

  FontFamily *default_font;

  bool error_mode;
  String fatal_error;
  String traceback;

  bool key_state[349];
  bool prev_key_state[349];

  bool mouse_state[3];
  bool prev_mouse_state[3];
  float prev_mouse_x;
  float prev_mouse_y;
  float mouse_x;
  float mouse_y;
  float scroll_x;
  float scroll_y;

  void *miniaudio_vfs;
  ma_engine audio_engine;
  Array<Sound *> garbage_sounds;
};

extern App *g_app;

inline void fatal_error(String str) {
  if (!g_app->error_mode) {
    g_app->fatal_error = to_cstr(str);
    fprintf(stderr, "%s\n", g_app->fatal_error.data);
    g_app->error_mode = true;
  }
}
