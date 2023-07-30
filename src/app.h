#pragma once

#include "audio.h"
#include "font.h"
#include "draw.h"

struct Module {
  String name;
  u64 modtime;
  i32 ref;
};

struct App {
  u64 time_begin;
  double delta_time;

  bool mounted;
  bool hot_reload_enabled;
  float reload_time_elapsed;
  float reload_interval;
  Archive archive;
  HashMap<Module> modules;

  FontFamily *default_font;
  bool default_font_loaded;

  float clear_color[4];
  Color draw_colors[32];
  u64 draw_colors_len;

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

  HashMap<Sprite> sprites;
};

extern App *g_app;