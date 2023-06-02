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

using TilemapInt = char;

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

struct TilemapLevel;
struct TilemapNeighbor {
  String iid;
  TilemapDir dir;
  TilemapLevel *level;
};

struct TilemapLevel {
  bool visited;
  String identifier;
  String iid;
  float world_x, world_y;
  float px_width, px_height;
  Array<TilemapLayer> layers;
  Array<TilemapNeighbor> neighbors;
};

struct TilemapGrid {
  HashMap<TilemapInt> values;
  float grid_size;
  u64 count;
};

struct Tilemap {
  Array<TilemapLevel> levels;
  HashMap<Image> images;
  HashMap<TilemapGrid> grids_by_layer;
  TilemapGrid *current_grid;
};

bool tilemap_load(Tilemap *tm, Archive *ar, String filepath);
void drop(Tilemap *tm);
void tilemap_grid_begin(Tilemap *tm, String layer);
void tilemap_grid_end(Tilemap *tm);
TilemapInt tilemap_grid_value(Tilemap *tm, float x, float y);
bool tilemap_rect_every(Tilemap *tm, TilemapInt needle, float x0, float y0,
                        float x1, float y1);
bool tilemap_rect_has(Tilemap *tm, TilemapInt needle, float x0, float y0,
                      float x1, float y1);
