#pragma once

#include "hash_map.h"
#include "image.h"
#include "priority_queue.h"
#include "slice.h"

struct Tile {
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
  Slice<Tile> tiles;
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
  Slice<TileNode *> neighbors;
};

struct TileCost {
  TilemapInt cell;
  float value;
};

struct TilePoint {
  float x, y;
};

inline u64 tile_key(i32 x, i32 y) { return ((u64)x << 32) | (u64)y; }

class b2Body;
class b2World;

struct Tilemap {
  Arena arena;
  Slice<TilemapLevel> levels;
  HashMap<Image> images;    // key: filepath
  HashMap<b2Body *> bodies; // key: layer name
  HashMap<TileNode> graph;  // key: x, y
  PriorityQueue<TileNode *> frontier;
  float graph_grid_size;

  bool load(String filepath);
  void trash();
  void destroy_bodies(b2World *world);
  void make_collision(b2World *world, float meter, String layer_name,
                      Slice<TilemapInt> walls);
  void make_graph(i32 bloom, String layer_name, Slice<TileCost> costs);
  TileNode *astar(TilePoint start, TilePoint goal);
};
