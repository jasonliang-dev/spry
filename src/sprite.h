#pragma once

#include "hash_map.h"
#include "image.h"
#include "slice.h"

struct SpriteFrame {
  i32 duration;
  float u0, v0, u1, v1;
};

struct SpriteLoop {
  Slice<i32> indices;
};

struct SpriteData {
  Arena arena;
  Slice<SpriteFrame> frames;
  HashMap<SpriteLoop> by_tag;
  Image img;
  i32 width;
  i32 height;

  bool load(String filepath);
  void trash();
};

struct Sprite {
  u64 sprite; // index into assets
  u64 loop;   // index into SpriteData::by_tag
  float elapsed;
  i32 current_frame;

  bool play(String tag);
  void update(float dt);
  void set_frame(i32 frame);
};

struct SpriteView {
  Sprite *sprite;
  SpriteData data;
  SpriteLoop loop;

  bool make(Sprite *spr);
  i32 frame();
  u64 len();
};
