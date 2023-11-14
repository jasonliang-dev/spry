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
};

struct Sprite {
  u64 sprite;
  u64 loop;
  float elapsed;
  i32 current_frame;
};

struct SpriteView {
  Sprite *sprite;
  const SpriteData *data;
  SpriteLoop *loop;
};

bool sprite_data_load(SpriteData *spr, Archive *ar, String filepath);
void sprite_data_trash(SpriteData *spr);

bool sprite_play(Sprite *spr, String tag);
void sprite_update(Sprite *spr, float dt);
void sprite_set_frame(Sprite *spr, i32 frame);

bool sprite_view(SpriteView *out, Sprite *spr);
void sprite_view_unlock();
i32 sprite_view_frame(SpriteView *view);
u64 sprite_view_len(SpriteView *view);
