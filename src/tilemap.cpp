#include "tilemap.h"
#include "arena.h"
#include "hash_map.h"
#include "json.h"
#include "prelude.h"
#include "priority_queue.h"
#include "profile.h"
#include "slice.h"
#include "strings.h"
#include "vfs.h"
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>

static bool layer_from_json(TilemapLayer *layer, JSON *json, bool *ok,
                            Arena *arena, String filepath,
                            HashMap<Image> *images) {
  PROFILE_FUNC();

  layer->identifier =
      arena->bump_string(json->lookup_string("__identifier", ok));
  layer->c_width = (i32)json->lookup_number("__cWid", ok);
  layer->c_height = (i32)json->lookup_number("__cHei", ok);
  layer->grid_size = json->lookup_number("__gridSize", ok);

  JSON tileset_rel_path = json->lookup("__tilesetRelPath", ok);

  JSONArray *int_grid_csv = json->lookup_array("intGridCsv", ok);

  JSONArray *grid_tiles = json->lookup_array("gridTiles", ok);
  JSONArray *auto_layer_tiles = json->lookup_array("autoLayerTiles", ok);

  JSONArray *arr_tiles = (grid_tiles != nullptr && grid_tiles->index != 0)
                             ? grid_tiles
                             : auto_layer_tiles;

  JSONArray *entity_instances = json->lookup_array("entityInstances", ok);

  if (tileset_rel_path.kind == JSONKind_String) {
    StringBuilder sb = {};
    defer(sb.trash());
    sb.swap_filename(filepath, tileset_rel_path.as_string(ok));

    u64 key = fnv1a(sb);

    Image *img = images->get(key);
    if (img != nullptr) {
      layer->image = *img;
    } else {
      Image create_img = {};
      bool success = create_img.load(sb);
      if (!success) {
        return false;
      }

      layer->image = create_img;
      (*images)[key] = create_img;
    }
  }

  Slice<TilemapInt> grid = {};
  if (int_grid_csv != nullptr) {
    PROFILE_BLOCK("int grid");

    i32 len = int_grid_csv->index + 1;
    grid.resize(arena, len);
    for (JSONArray *a = int_grid_csv; a != nullptr; a = a->next) {
      grid[--len] = (TilemapInt)a->value.as_number(ok);
    }
  }
  layer->int_grid = grid;

  Slice<Tile> tiles = {};
  if (arr_tiles != nullptr) {
    PROFILE_BLOCK("tiles");

    i32 len = arr_tiles->index + 1;
    tiles.resize(arena, len);
    for (JSONArray *a = arr_tiles; a != nullptr; a = a->next) {
      JSON px = a->value.lookup("px", ok);
      JSON src = a->value.lookup("src", ok);

      Tile tile = {};
      tile.x = px.index_number(0, ok);
      tile.y = px.index_number(1, ok);

      tile.u = src.index_number(0, ok);
      tile.v = src.index_number(1, ok);

      tile.flip_bits = (i32)a->value.lookup_number("f", ok);
      tiles[--len] = tile;
    }
  }
  layer->tiles = tiles;

  for (Tile &tile : layer->tiles) {
    tile.u0 = tile.u / layer->image.width;
    tile.v0 = tile.v / layer->image.height;
    tile.u1 = (tile.u + layer->grid_size) / layer->image.width;
    tile.v1 = (tile.v + layer->grid_size) / layer->image.height;

    i32 FLIP_X = 1 << 0;
    i32 FLIP_Y = 1 << 1;

    if (tile.flip_bits & FLIP_X) {
      float tmp = tile.u0;
      tile.u0 = tile.u1;
      tile.u1 = tmp;
    }

    if (tile.flip_bits & FLIP_Y) {
      float tmp = tile.v0;
      tile.v0 = tile.v1;
      tile.v1 = tmp;
    }
  }

  Slice<TilemapEntity> entities = {};
  if (entity_instances != nullptr) {
    PROFILE_BLOCK("entities");

    i32 len = entity_instances->index + 1;
    entities.resize(arena, len);
    for (JSONArray *a = entity_instances; a != nullptr; a = a->next) {
      JSON px = a->value.lookup("px", ok);

      TilemapEntity entity = {};
      entity.x = px.index_number(0, ok);
      entity.y = px.index_number(1, ok);
      entity.identifier =
          arena->bump_string(a->value.lookup_string("__identifier", ok));

      entities[--len] = entity;
    }
  }
  layer->entities = entities;

  return true;
}

static bool level_from_json(TilemapLevel *level, JSON *json, bool *ok,
                            Arena *arena, String filepath,
                            HashMap<Image> *images) {
  PROFILE_FUNC();

  level->identifier = arena->bump_string(json->lookup_string("identifier", ok));
  level->iid = arena->bump_string(json->lookup_string("iid", ok));
  level->world_x = json->lookup_number("worldX", ok);
  level->world_y = json->lookup_number("worldY", ok);
  level->px_width = json->lookup_number("pxWid", ok);
  level->px_height = json->lookup_number("pxHei", ok);

  JSONArray *layer_instances = json->lookup_array("layerInstances", ok);

  Slice<TilemapLayer> layers = {};
  if (layer_instances != nullptr) {
    i32 len = layer_instances->index + 1;
    layers.resize(arena, len);
    for (JSONArray *a = layer_instances; a != nullptr; a = a->next) {
      TilemapLayer layer = {};
      bool success =
          layer_from_json(&layer, &a->value, ok, arena, filepath, images);
      if (!success) {
        return false;
      }
      layers[--len] = layer;
    }
  }
  level->layers = layers;

  return true;
}

bool Tilemap::load(String filepath) {
  PROFILE_FUNC();

  String contents = {};
  bool success = vfs_read_entire_file(&contents, filepath);
  if (!success) {
    return false;
  }
  defer(mem_free(contents.data));

  bool ok = true;
  JSONDocument doc = {};
  doc.parse(contents);
  defer(doc.trash());

  if (doc.error.len != 0) {
    return false;
  }

  Arena arena = {};
  HashMap<Image> images = {};
  bool created = false;
  defer({
    if (!created) {
      for (auto [k, v] : images) {
        v->trash();
      }
      images.trash();
      arena.trash();
    }
  });

  JSONArray *arr_levels = doc.root.lookup_array("levels", &ok);

  Slice<TilemapLevel> levels = {};
  if (arr_levels != nullptr) {
    i32 len = arr_levels->index + 1;
    levels.resize(&arena, len);
    for (JSONArray *a = arr_levels; a != nullptr; a = a->next) {
      TilemapLevel level = {};
      bool success =
          level_from_json(&level, &a->value, &ok, &arena, filepath, &images);
      if (!success) {
        return false;
      }
      levels[--len] = level;
    }
  }

  if (!ok) {
    return false;
  }

  Tilemap tilemap = {};
  tilemap.arena = arena;
  tilemap.levels = levels;
  tilemap.images = images;

  printf("loaded tilemap with %llu levels\n",
         (unsigned long long)tilemap.levels.len);
  *this = tilemap;
  created = true;
  return true;
}

void Tilemap::trash() {
  for (auto [k, v] : images) {
    v->trash();
  }
  images.trash();

  bodies.trash();
  graph.trash();
  frontier.trash();

  arena.trash();
}

void Tilemap::destroy_bodies(b2World *world) {
  for (auto [k, v] : bodies) {
    world->DestroyBody(*v);
  }
}

static void make_collision_for_layer(b2Body *body, TilemapLayer *layer,
                                     float world_x, float world_y, float meter,
                                     Slice<TilemapInt> walls) {
  PROFILE_FUNC();

  auto is_wall = [layer, walls](i32 y, i32 x) {
    if (x >= layer->c_width || y >= layer->c_height) {
      return false;
    }

    for (TilemapInt n : walls) {
      if (layer->int_grid[y * layer->c_width + x] == n) {
        return true;
      }
    }

    return false;
  };

  Array<bool> filled = {};
  defer(filled.trash());
  filled.resize(layer->c_width * layer->c_height);
  memset(filled.data, 0, layer->c_width * layer->c_height);
  for (i32 y = 0; y < layer->c_height; y++) {
    for (i32 x = 0; x < layer->c_width; x++) {
      i32 x0 = x;
      i32 y0 = y;
      i32 x1 = x;
      i32 y1 = y;

      if (!is_wall(y1, x1)) {
        continue;
      }

      if (filled[y1 * layer->c_width + x1]) {
        continue;
      }

      while (is_wall(y1, x1 + 1)) {
        x1++;
      }

      while (true) {
        bool walkable = false;
        for (i32 x = x0; x <= x1; x++) {
          if (!is_wall(y1 + 1, x)) {
            walkable = true;
          }
        }

        if (walkable) {
          break;
        }

        y1++;
      }

      for (i32 y = y0; y <= y1; y++) {
        for (i32 x = x0; x <= x1; x++) {
          filled[y * layer->c_width + x] = true;
        }
      }

      float dx = (float)(x1 + 1 - x0) * layer->grid_size / 2.0f;
      float dy = (float)(y1 + 1 - y0) * layer->grid_size / 2.0f;

      b2Vec2 pos = {
          (x0 * layer->grid_size + dx + world_x) / meter,
          (y0 * layer->grid_size + dy + world_y) / meter,
      };

      b2PolygonShape box = {};
      box.SetAsBox(dx / meter, dy / meter, pos, 0.0f);

      b2FixtureDef def = {};
      def.friction = 0;
      def.shape = &box;

      body->CreateFixture(&def);
    }
  }
}

void Tilemap::make_collision(b2World *world, float meter, String layer_name,
                             Slice<TilemapInt> walls) {
  PROFILE_FUNC();

  b2Body *body = nullptr;
  {
    b2BodyDef def = {};
    def.position.x = 0;
    def.position.y = 0;
    def.fixedRotation = true;
    def.allowSleep = true;
    def.awake = false;
    def.type = b2_staticBody;
    def.gravityScale = 0;

    body = world->CreateBody(&def);
  }

  for (TilemapLevel &level : levels) {
    for (TilemapLayer &l : level.layers) {
      if (l.identifier == layer_name) {
        make_collision_for_layer(body, &l, level.world_x, level.world_y, meter,
                                 walls);
      }
    }
  }

  bodies[fnv1a(layer_name)] = body;
}

static float get_tile_cost(TilemapInt n, Slice<TileCost> costs) {
  for (TileCost cost : costs) {
    if (cost.cell == n) {
      return cost.value;
    }
  }
  return -1;
}

static void make_graph_for_layer(HashMap<TileNode> *graph, TilemapLayer *layer,
                                 float world_x, float world_y,
                                 Slice<TileCost> costs) {
  PROFILE_FUNC();

  for (i32 y = 0; y < layer->c_height; y++) {
    for (i32 x = 0; x < layer->c_width; x++) {
      float cost =
          get_tile_cost(layer->int_grid[y * layer->c_width + x], costs);
      if (cost > 0) {
        TileNode node = {};
        node.x = (i32)(x + world_x);
        node.y = (i32)(y + world_x);
        node.cost = cost;

        (*graph)[tile_key(node.x, node.y)] = node;
      }
    }
  }
}

static bool tilemap_rect_overlaps_graph(HashMap<TileNode> *graph, i32 x0,
                                        i32 y0, i32 x1, i32 y1) {
  i32 lhs = x0 <= x1 ? x0 : x1;
  i32 rhs = x0 <= x1 ? x1 : x0;
  i32 top = y0 <= y1 ? y0 : y1;
  i32 bot = y0 <= y1 ? y1 : y0;

  for (i32 y = top; y <= bot; y++) {
    for (i32 x = lhs; x <= rhs; x++) {
      if ((x == x0 && y == y0) || (x == x1 && y == y1)) {
        continue;
      }

      TileNode *node = graph->get(tile_key(x, y));
      if (node == nullptr) {
        return false;
      }
    }
  }

  return true;
}

static void create_neighbor_nodes(HashMap<TileNode> *graph, Arena *arena,
                                  i32 bloom) {
  PROFILE_FUNC();

  for (auto [k, v] : *graph) {
    i32 len = 0;
    Slice<TileNode *> neighbors = {};

    for (i32 y = -bloom; y <= bloom; y++) {
      for (i32 x = -bloom; x <= bloom; x++) {
        if (x == 0 && y == 0) {
          continue;
        }

        i32 dx = v->x + x;
        i32 dy = v->y + y;
        TileNode *node = graph->get(tile_key(dx, dy));
        if (node != nullptr) {
          bool ok = tilemap_rect_overlaps_graph(graph, v->x, v->y, dx, dy);
          if (!ok) {
            continue;
          }

          if (len == neighbors.len) {
            i32 grow = len > 0 ? len * 2 : 8;
            neighbors.resize(arena, grow);
          }

          neighbors[len] = node;
          len++;
        }
      }
    }

    neighbors.resize(arena, len);
    v->neighbors = neighbors;
  }
}

void Tilemap::make_graph(i32 bloom, String layer_name, Slice<TileCost> costs) {
  for (TilemapLevel &level : levels) {
    for (TilemapLayer &l : level.layers) {
      if (l.identifier == layer_name) {
        if (graph_grid_size == 0) {
          graph_grid_size = l.grid_size;
        }
        make_graph_for_layer(&graph, &l, level.world_x, level.world_y, costs);
      }
    }
  }

  create_neighbor_nodes(&graph, &arena, bloom);
}

static float tile_distance(TileNode *lhs, TileNode *rhs) {
  float dx = lhs->x - rhs->x;
  float dy = lhs->y - rhs->y;
  return sqrtf(dx * dx + dy * dy);
}

static float tile_heuristic(TileNode *lhs, TileNode *rhs) {
  float D = 1;
  float D2 = 1.4142135f;

  float dx = (float)abs(lhs->x - rhs->x);
  float dy = (float)abs(lhs->y - rhs->y);
  return D * (dx + dy) + (D2 - 2 * D) * fminf(dx, dy);
}

static void astar_reset(Tilemap *tm) {
  PROFILE_FUNC();

  tm->frontier.len = 0;

  for (auto [k, v] : tm->graph) {
    v->prev = nullptr;
    v->g = 0;
    v->flags = 0;
  }
}

TileNode *Tilemap::astar(TilePoint start, TilePoint goal) {
  PROFILE_FUNC();

  astar_reset(this);

  i32 sx = (i32)(start.x / graph_grid_size);
  i32 sy = (i32)(start.y / graph_grid_size);
  i32 ex = (i32)(goal.x / graph_grid_size);
  i32 ey = (i32)(goal.y / graph_grid_size);

  TileNode *end = graph.get(tile_key(ex, ey));
  if (end == nullptr) {
    return nullptr;
  }

  TileNode *begin = graph.get(tile_key(sx, sy));
  if (begin == nullptr) {
    return nullptr;
  }

  float g = 0;
  float h = tile_heuristic(begin, end);
  float f = g + h;
  begin->g = 0;
  begin->flags |= TileNodeFlags_Open;
  frontier.push(begin, f);

  while (frontier.len != 0) {
    TileNode *top = nullptr;
    frontier.pop(&top);
    top->flags |= TileNodeFlags_Closed;

    if (top == end) {
      return top;
    }

    for (TileNode *next : top->neighbors) {
      if (next->flags & TileNodeFlags_Closed) {
        continue;
      }

      float g = top->g + next->cost * tile_distance(top, next);

      bool open = next->flags & TileNodeFlags_Open;
      if (!open || g < next->g) {
        float h = tile_heuristic(next, end);
        float f = g + h;

        next->g = g;
        next->prev = top;
        next->flags |= TileNodeFlags_Open;

        frontier.push(next, f);
      }
    }
  }

  return nullptr;
}
