#pragma once

#include "assets.h"
#include "audio.h"
#include "deps/lua/lua.h"
#include "deps/luaalloc.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "draw.h"

struct AppTime {
  u64 last;
  u64 accumulator;
  u64 target_ticks;
  double delta;
};

struct FileChange {
  u64 key;
  u64 modtime;
};

struct App {
  Archive *archive;
  Assets assets;

  struct {
    cute_atomic_int_t shutdown_request;
    cute_thread_t *thread;
    Array<FileChange> changes;
  } hot_reload;

  cute_mutex_t lua_mtx;
  LuaAlloc *LA;
  lua_State *L;

  sgl_pipeline pipeline;

  AppTime time;

  bool hot_reload_enabled;
  float reload_time_elapsed;
  float reload_interval;

  FontFamily *default_font;

  Renderer2D renderer;

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

  ma_engine audio_engine;
  Array<Sound *> garbage_sounds;
};

extern App *g_app;