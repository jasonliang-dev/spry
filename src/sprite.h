#pragma once

#include "image.h"
#include "array.h"
#include "hash_map.h"

struct SpriteFrame {
  i32 duration;
  float u0, v0, u1, v1;
};

struct SpriteLoop {
  Array<i32> indices;
};

struct Sprite;

struct SpriteRenderer {
  Sprite *sprite;
  SpriteLoop *loop;
  float elapsed;
  i32 current_frame;
};

struct Sprite {
  Array<SpriteFrame> frames;
  HashMap<SpriteLoop> by_tag;
  Image img;
  i32 width;
  i32 height;
};

bool sprite_load(Sprite *spr, Archive *ar, String filepath);
void drop(Sprite *spr);
void sprite_renderer_play(SpriteRenderer *sr, String tag);
void sprite_renderer_update(SpriteRenderer *sr, float dt);
SpriteFrame sprite_renderer_frame(SpriteRenderer *sr);
void sprite_renderer_set_frame(SpriteRenderer *sr, i32 frame);
