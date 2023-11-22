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

static bool layer_from_json(TilemapLayer *layer, JSON *json, Arena *arena,
                            String filepath, HashMap<Image> *images) {
  PROFILE_FUNC();

  layer->identifier =
      arena_bump_string(arena, json_lookup_string(json, "__identifier"));
  layer->c_width = (i32)json_lookup_number(json, "__cWid");
  layer->c_height = (i32)json_lookup_number(json, "__cHei");
  layer->grid_size = json_lookup_number(json, "__gridSize");

  JSON *tileset_rel_path = json_lookup(json, "__tilesetRelPath");
  if (tileset_rel_path == nullptr) {
    return false;
  }

  JSONArray *int_grid_csv = json_array(json_lookup(json, "intGridCsv"));

  JSONArray *grid_tiles = json_array(json_lookup(json, "gridTiles"));
  JSONArray *auto_layer_tiles = json_array(json_lookup(json, "autoLayerTiles"));

  JSONArray *arr_tiles = (grid_tiles != nullptr && grid_tiles->index != 0)
                             ? grid_tiles
                             : auto_layer_tiles;

  JSONArray *entity_instances =
      json_array(json_lookup(json, "entityInstances"));

  if (tileset_rel_path->kind == JSONKind_String) {
    StringBuilder sb = string_builder_make();
    defer(string_builder_trash(&sb));

    string_builder_swap_filename(&sb, filepath, json_string(tileset_rel_path));

    String fullpath = string_builder_as_string(&sb);
    u64 key = fnv1a(fullpath);

    Image *img = hashmap_get(images, key);
    if (img != nullptr) {
      layer->image = *img;
    } else {
      Image create_img = {};
      bool ok = image_load(&create_img, fullpath);
      if (!ok) {
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
    slice_from_arena(&grid, arena, len);
    for (JSONArray *a = int_grid_csv; a != nullptr; a = a->next) {
      grid[--len] = (TilemapInt)json_number(&a->value);
    }
  }
  layer->int_grid = grid;

  Slice<Tile> tiles = {};
  if (arr_tiles != nullptr) {
    PROFILE_BLOCK("tiles");

    i32 len = arr_tiles->index + 1;
    slice_from_arena(&tiles, arena, len);
    for (JSONArray *a = arr_tiles; a != nullptr; a = a->next) {
      JSON *px = json_lookup(&a->value, "px");
      JSON *src = json_lookup(&a->value, "src");

      Tile tile = {};
      tile.x = json_number(json_index(px, 0));
      tile.y = json_number(json_index(px, 1));

      tile.u = json_number(json_index(src, 0));
      tile.v = json_number(json_index(src, 1));

      tile.flip_bits = (i32)json_lookup_number(&a->value, "f");
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
    slice_from_arena(&entities, arena, len);
    for (JSONArray *a = entity_instances; a != nullptr; a = a->next) {
      JSON *px = json_lookup(&a->value, "px");

      TilemapEntity entity = {};
      entity.x = json_number(json_index(px, 0));
      entity.y = json_number(json_index(px, 1));
      entity.identifier = arena_bump_string(
          arena, json_lookup_string(&a->value, "__identifier"));

      entities[--len] = entity;
    }
  }
  layer->entities = entities;

  return true;
}

static bool level_from_json(TilemapLevel *level, JSON *json, Arena *arena,
                            String filepath, HashMap<Image> *images) {
  PROFILE_FUNC();

  level->identifier =
      arena_bump_string(arena, json_lookup_string(json, "identifier"));
  level->iid = arena_bump_string(arena, json_lookup_string(json, "iid"));
  level->world_x = json_lookup_number(json, "worldX");
  level->world_y = json_lookup_number(json, "worldY");
  level->px_width = json_lookup_number(json, "pxWid");
  level->px_height = json_lookup_number(json, "pxHei");

  JSONArray *layer_instances = json_array(json_lookup(json, "layerInstances"));

  Slice<TilemapLayer> layers = {};
  if (layer_instances != nullptr) {
    i32 len = layer_instances->index + 1;
    slice_from_arena(&layers, arena, len);
    for (JSONArray *a = layer_instances; a != nullptr; a = a->next) {
      TilemapLayer layer = {};
      bool ok = layer_from_json(&layer, &a->value, arena, filepath, images);
      if (!ok) {
        return false;
      }
      layers[--len] = layer;
    }
  }
  level->layers = layers;

  return true;
}

bool tilemap_load(Tilemap *tm, String filepath) {
  PROFILE_FUNC();

  String contents = {};
  bool ok = vfs_read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  JSONDocument doc = {};
  json_parse(&doc, contents);
  defer(json_trash(&doc));

  if (doc.error.len != 0) {
    return false;
  }

  Arena arena = {};
  HashMap<Image> images = {};
  bool success = false;
  defer({
    if (!success) {
      for (auto [k, v] : images) {
        image_trash(v);
      }
      hashmap_trash(&images);
      arena_trash(&arena);
    }
  });

  JSONArray *arr_levels = json_array(json_lookup(&doc.root, "levels"));

  Slice<TilemapLevel> levels = {};
  if (arr_levels != nullptr) {
    i32 len = arr_levels->index + 1;
    slice_from_arena(&levels, &arena, len);
    for (JSONArray *a = arr_levels; a != nullptr; a = a->next) {
      TilemapLevel level = {};
      bool ok = level_from_json(&level, &a->value, &arena, filepath, &images);
      if (!ok) {
        return false;
      }
      levels[--len] = level;
    }
  }

  Tilemap tilemap = {};
  tilemap.arena = arena;
  tilemap.levels = levels;
  tilemap.images = images;

  if (doc.root.had_error) {
    return false;
  }

  printf("loaded tilemap with %llu levels\n",
         (unsigned long long)tilemap.levels.len);
  *tm = tilemap;
  success = true;
  return true;
}

void tilemap_trash(Tilemap *tm) {
  for (auto [k, v] : tm->images) {
    image_trash(v);
  }
  hashmap_trash(&tm->images);

  hashmap_trash(&tm->bodies);
  hashmap_trash(&tm->graph);
  priority_queue_trash(&tm->frontier);

  arena_trash(&tm->arena);
}

void tilemap_destroy_bodies(Tilemap *tm, b2World *world) {
  for (auto [k, v] : tm->bodies) {
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
  defer(array_trash(&filled));
  array_resize(&filled, layer->c_width * layer->c_height);
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

void tilemap_make_collision(Tilemap *tm, b2World *world, float meter,
                            String layer_name, Slice<TilemapInt> walls) {
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

  for (TilemapLevel &level : tm->levels) {
    for (TilemapLayer &l : level.layers) {
      if (l.identifier == layer_name) {
        make_collision_for_layer(body, &l, level.world_x, level.world_y, meter,
                                 walls);
      }
    }
  }

  tm->bodies[fnv1a(layer_name)] = body;
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

      TileNode *node = hashmap_get(graph, tile_key(x, y));
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
        TileNode *node = hashmap_get(graph, tile_key(dx, dy));
        if (node != nullptr) {
          bool ok = tilemap_rect_overlaps_graph(graph, v->x, v->y, dx, dy);
          if (!ok) {
            continue;
          }

          if (len == neighbors.len) {
            i32 grow = len > 0 ? len * 2 : 8;
            slice_resize(&neighbors, arena, grow);
          }

          neighbors[len] = node;
          len++;
        }
      }
    }

    slice_resize(&neighbors, arena, len);
    v->neighbors = neighbors;
  }
}

void tilemap_make_graph(Tilemap *tm, i32 bloom, String layer_name,
                        Slice<TileCost> costs) {
  HashMap<TileNode> graph = {};
  float grid_size = 0;

  for (TilemapLevel &level : tm->levels) {
    for (TilemapLayer &l : level.layers) {
      if (l.identifier == layer_name) {
        if (grid_size == 0) {
          grid_size = l.grid_size;
        }
        make_graph_for_layer(&graph, &l, level.world_x, level.world_y, costs);
      }
    }
  }

  create_neighbor_nodes(&graph, &tm->arena, bloom);

  tm->graph = graph;
  tm->graph_grid_size = grid_size;
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

TileNode *tilemap_astar(Tilemap *tm, TilePoint start, TilePoint goal) {
  PROFILE_FUNC();

  astar_reset(tm);

  i32 sx = (i32)(start.x / tm->graph_grid_size);
  i32 sy = (i32)(start.y / tm->graph_grid_size);
  i32 ex = (i32)(goal.x / tm->graph_grid_size);
  i32 ey = (i32)(goal.y / tm->graph_grid_size);

  TileNode *end = hashmap_get(&tm->graph, tile_key(ex, ey));
  if (end == nullptr) {
    return nullptr;
  }

  TileNode *begin = hashmap_get(&tm->graph, tile_key(sx, sy));
  if (begin == nullptr) {
    return nullptr;
  }

  float g = 0;
  float h = tile_heuristic(begin, end);
  float f = g + h;
  begin->g = 0;
  begin->flags |= TileNodeFlags_Open;
  priority_queue_push(&tm->frontier, begin, f);

  while (tm->frontier.len != 0) {
    TileNode *top = nullptr;
    priority_queue_pop(&tm->frontier, &top);
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

        priority_queue_push(&tm->frontier, next, f);
      }
    }
  }

  return nullptr;
}
