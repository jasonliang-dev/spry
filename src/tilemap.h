#pragma once

#include "hash_map.h"
#include "image.h"

struct TilemapTile {
  float x, y, u, v;
  float u0, v0, u1, v1;
  i32 flip_bits;
};

struct TilemapEntity {
  String identifier;
  float x, y;
};

union TilemapPoint {
  struct {
    i32 x;
    i32 y;
  };
  u64 value;
};

using TilemapInt = unsigned char;

struct TilemapLayer {
  String identifier;
  Image image;
  Array<TilemapTile> tiles;
  Array<TilemapEntity> entities;
  i32 c_width;
  i32 c_height;
  Array<TilemapInt> int_grid;
  float grid_size;
};

enum TilemapDir : i32 {
  TilemapDir_North,
  TilemapDir_East,
  TilemapDir_South,
  TilemapDir_West,
};

struct TilemapLevel {
  String identifier;
  String iid;
  float world_x, world_y;
  float px_width, px_height;
  Array<TilemapLayer> layers;
};

class b2Body;
struct Tilemap {
  Array<TilemapLevel> levels;
  HashMap<Image> images; // key: filepath
  HashMap<b2Body *> bodies; // key: layer name
};

bool tilemap_load(Tilemap *tm, Archive *ar, String filepath);
void drop(Tilemap *tm);

class b2World;
void tilemap_make_collision(Tilemap *tm, b2World *world, float meter,
                            String layer_name, Array<TilemapInt> *walls);
