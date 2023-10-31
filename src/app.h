#pragma once

#include "audio.h"
#include "font.h"
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

struct App {
  u64 time_begin;
  double delta_time;

  bool hot_reload_enabled;
  float reload_time_elapsed;
  float reload_interval;
  Archive archive;
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

  float master_volume;
  Array<float> audio_buffer;
  AudioSources audio_sources;

  HashMap<Asset> assets;
};

extern App *g_app;