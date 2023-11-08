#pragma once

#include "hash_map.h"
#include "image.h"
#include "priority_queue.h"
#include "slice.h"

struct TilemapTile {
  float x, y, u, v;
  float u0, v0, u1, v1;
  i32 flip_bits;
};

struct TilemapEntity {
  String identifier;
  float x, y;
};

using TilemapInt = unsigned char;

struct TilemapLayer {
  String identifier;
  Image image;
  Slice<TilemapTile> tiles;
  Slice<TilemapEntity> entities;
  i32 c_width;
  i32 c_height;
  Slice<TilemapInt> int_grid;
  float grid_size;
};

struct TilemapLevel {
  String identifier;
  String iid;
  float world_x, world_y;
  float px_width, px_height;
  Slice<TilemapLayer> layers;
};

enum TileNodeFlags {
  TileNodeFlags_Open = 1 << 0,
  TileNodeFlags_Closed = 1 << 1,
};

struct TileNode {
  TileNode *prev;
  float g; // cost so far
  u32 flags;

  i32 x, y;
  float cost;
  i32 neighbor_count;
  TileNode *neighbors[8];
};

struct TileCost {
  TilemapInt cell;
  float value;
};

inline u64 tile_key(i32 x, i32 y) { return ((u64)x << 32) | (u64)y; }

struct TilePoint {
  i32 x, y;
};

class b2Body;
class b2World;

struct Tilemap {
  Arena arena;
  Slice<TilemapLevel> levels;
  HashMap<Image> images;    // key: filepath
  HashMap<b2Body *> bodies; // key: layer name
  HashMap<TileNode> graph;  // key: x, y
  PriorityQueue<TileNode *> frontier;
};

bool tilemap_load(Tilemap *tm, Archive *ar, String filepath);
void tilemap_trash(Tilemap *tm);
void tilemap_destroy_bodies(Tilemap *tm, b2World *world);

void tilemap_make_collision(Tilemap *tm, b2World *world, float meter,
                            String layer_name, Slice<TilemapInt> walls);
void tilemap_make_graph(Tilemap *tm, String layer_name, Slice<TileCost> costs);
TileNode *tilemap_astar(Tilemap *tm, TilePoint start, TilePoint goal);
