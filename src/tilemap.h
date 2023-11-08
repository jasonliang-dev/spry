#pragma once

#include "hash_map.h"
#include "image.h"
#include "priority_queue.h"

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
  Array<TilemapTile> tiles;
  Array<TilemapEntity> entities;
  i32 c_width;
  i32 c_height;
  Array<TilemapInt> int_grid;
  float grid_size;
};

struct TilemapLevel {
  String identifier;
  String iid;
  float world_x, world_y;
  float px_width, px_height;
  Array<TilemapLayer> layers;
};

enum TileFlags : i32 {
  TileFlags_Open = 1 << 0,
  TileFlags_Closed = 2 << 0,
};

struct TileNode {
  TileNode *parent;
  TileFlags flags;
  i32 x, y;
  float cost;
  float f;
  float g;
  float h;
  TileNode *neighbors[8];
  i32 neighbor_count;
};

struct TileCost {
  TilemapInt cell;
  float value;
};

struct TilePoint {
  i32 x, y;
};

class b2Body;
class b2World;

struct Tilemap {
  Array<TilemapLevel> levels;
  HashMap<Image> images;    // key: filepath
  HashMap<b2Body *> bodies; // key: layer name
  HashMap<TileNode> graph;  // key: x, y
  PriorityQueue<TileNode *> frontier;
};

bool tilemap_load(Tilemap *tm, Archive *ar, String filepath);
void tilemap_trash(Tilemap *tm);
void tilemap_destroy_bodies(Tilemap *tm, b2World *world);

void tilemap_make_collision(Tilemap *tm, b2World *world, float meter,
                            String layer_name, Array<TilemapInt> *walls);
void tilemap_make_graph(Tilemap *tm, String layer_name, Array<TileCost> *costs);
void tilemap_astar(Tilemap *tm, Array<TilePoint> *out, TilePoint start,
                   TilePoint end);