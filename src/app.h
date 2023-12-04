#pragma once

#include "array.h"
#include "concurrency.h"
#include "deps/luaalloc.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "font.h"
#include "slice.h"
#include "sound.h"

#define SPRY_VERSION "0.8"

struct AppTime {
  u64 last;
  u64 accumulator;
  u64 target_ticks;
  double delta;
};

struct lua_State;
struct App {
  Mutex frame_mtx;

  LuaAlloc *LA;
  lua_State *L;

  AppTime time;

  Slice<String> args;

  Mutex err_mtx;
  bool error_mode;
  String fatal_error;
  String traceback;

  AtomicInt hot_reload_enabled;
  AtomicInt reload_interval;

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

  FontFamily *default_font;

  void *miniaudio_vfs;
  ma_engine audio_engine;
  Array<Sound *> garbage_sounds;

  LuaChannels channels;
};

extern App *g_app;

inline void fatal_error(String str) {
  if (!g_app->error_mode) {
    g_app->fatal_error = to_cstr(str);
    fprintf(stderr, "%s\n", g_app->fatal_error.data);
    g_app->error_mode = true;
  }
}
