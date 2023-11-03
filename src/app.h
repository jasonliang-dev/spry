#pragma once

#include "deps/miniaudio.h"
#include "draw.h"

struct Module {
  char *name;
  u64 modtime;
  i32 ref;
};

enum AssetKind : u64 {
  AssetKind_None,
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
    Image image;
    Sprite sprite;
    Tilemap tilemap;
  };
};

struct AppTime {
  u64 last;
  double delta;
  double accumulator;
  double target_fps;
};

struct App {
  AppTime time;

  bool hot_reload_enabled;
  float reload_time_elapsed;
  float reload_interval;
  Archive *archive;
  HashMap<Module> modules;

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
  float mouse_dx;
  float mouse_dy;
  float mouse_x;
  float mouse_y;
  float scroll_x;
  float scroll_y;

  ma_engine audio_engine;

  HashMap<Asset> assets;
};

extern App *g_app;