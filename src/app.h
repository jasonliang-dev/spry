#pragma once

#include "audio.h"
#include "deps/lua/lua.h"
#include "deps/luaalloc.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "draw.h"
#include "os.h"

enum AssetKind : u64 {
  AssetKind_None,
  AssetKind_LuaRef,
  AssetKind_Image,
  AssetKind_Sprite,
  AssetKind_Tilemap,
};

struct Asset {
  char *name;
  u64 hash;
  u64 modtime;
  AssetKind kind;
  union {
    i32 lua_ref;
    Image image;
    SpriteData sprite;
    Tilemap tilemap;
  };
};

inline bool get_asset(HashMap<Asset> *assets, AssetKind kind, String filepath,
                      Asset **out) {
  Asset *asset = nullptr;
  u64 key = fnv1a(filepath);

  bool ok = hashmap_index(assets, key, &asset);
  if (!ok) {
    asset->name = to_cstr(filepath).data;
    asset->hash = key;
    asset->modtime = os_file_modtime(asset->name);
    asset->kind = kind;
  }

  if (asset->kind != kind) {
    return false;
  }

  *out = asset;
  return ok;
}

struct AppTime {
  u64 last;
  u64 accumulator;
  u64 target_ticks;
  double delta;
};

struct App {
  cute_mutex_t mtx;
  Archive *archive;
  HashMap<Asset> assets;
  LuaAlloc *LA;
  lua_State *L;

  sgl_pipeline pipeline;

  AppTime time;

  bool hot_reload_enabled;
  float reload_time_elapsed;
  float reload_interval;

  struct {
    cute_atomic_int_t shutdown_request;
    cute_thread_t *thread;
  } hot_reload;

  FontFamily *default_font;
  bool default_font_loaded;

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