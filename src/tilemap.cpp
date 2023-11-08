#include "tilemap.h"
#include "arena.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_world.h"
#include "hash_map.h"
#include "json.h"
#include "prelude.h"
#include "priority_queue.h"
#include "profile.h"
#include "slice.h"
#include "strings.h"

static bool layer_from_json(TilemapLayer *layer, JSON *json, Arena *arena,
                            Archive *ar, String filepath,
                            HashMap<Image> *images) {
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
      bool ok = image_load(&create_img, ar, fullpath);
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

  Slice<TilemapTile> tiles = {};
  if (arr_tiles != nullptr) {
    PROFILE_BLOCK("tiles");

    i32 len = arr_tiles->index + 1;
    slice_from_arena(&tiles, arena, len);
    for (JSONArray *a = arr_tiles; a != nullptr; a = a->next) {
      JSON *px = json_lookup(&a->value, "px");
      JSON *src = json_lookup(&a->value, "src");

      TilemapTile tile = {};
      tile.x = json_number(json_index(px, 0));
      tile.y = json_number(json_index(px, 1));

      tile.u = json_number(json_index(src, 0));
      tile.v = json_number(json_index(src, 1));

      tile.flip_bits = (i32)json_lookup_number(&a->value, "f");
      tiles[--len] = tile;
    }
  }
  layer->tiles = tiles;

  for (TilemapTile &tile : layer->tiles) {
    tile.u0 = tile.u / layer->image.width;
    tile.v0 = tile.v / layer->image.height;
    tile.u1 = (tile.u + layer->grid_size) / layer->image.width;
    tile.v1 = (tile.v + layer->grid_size) / layer->image.height;
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
                            Archive *ar, String filepath,
                            HashMap<Image> *images) {
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
      bool ok = layer_from_json(&layer, &a->value, arena, ar, filepath, images);
      if (!ok) {
        return false;
      }
      layers[--len] = layer;
    }
  }
  level->layers = layers;

  return true;
}

bool tilemap_load(Tilemap *tm, Archive *ar, String filepath) {
  PROFILE_FUNC();

  Arena arena = {};

  String contents = {};
  bool ok = ar->read_entire_file(&contents, filepath);
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

  HashMap<Image> images = {};
  Tilemap tilemap = {};

  JSONArray *arr_levels = json_array(json_lookup(&doc.root, "levels"));

  Slice<TilemapLevel> levels = {};
  if (arr_levels != nullptr) {
    i32 len = arr_levels->index + 1;
    slice_from_arena(&levels, &arena, len);
    for (JSONArray *a = arr_levels; a != nullptr; a = a->next) {
      TilemapLevel level = {};
      bool ok =
          level_from_json(&level, &a->value, &arena, ar, filepath, &images);
      if (!ok) {
        return false;
      }
      levels[--len] = level;
    }
  }

  tilemap.arena = arena;
  tilemap.levels = levels;
  tilemap.images = images;

  if (doc.root.had_error) {
    return false;
  }

  printf("loaded tilemap with %llu levels\n",
         (unsigned long long)tilemap.levels.len);
  *tm = tilemap;
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
        node.x = x + (i32)world_x;
        node.y = y + (i32)world_x;
        node.cost = cost;

        (*graph)[tile_key(node.x, node.y)] = node;
      }
    }
  }
}

void tilemap_make_graph(Tilemap *tm, String layer_name, Slice<TileCost> costs) {
  PROFILE_FUNC();

  HashMap<TileNode> graph = {};

  for (TilemapLevel &level : tm->levels) {
    for (TilemapLayer &l : level.layers) {
      if (l.identifier == layer_name) {
        make_graph_for_layer(&graph, &l, level.world_x, level.world_y, costs);
      }
    }
  }

  {
    PROFILE_BLOCK("create neighbors");

    for (auto [k, v] : graph) {
      i32 count = 0;

      for (i32 y = -1; y <= 1; y++) {
        for (i32 x = -1; x <= 1; x++) {
          if (x == 0 && y == 0) {
            continue;
          }

          i32 xx = v->x + x;
          i32 yy = v->y + y;
          TileNode *n = hashmap_get(&graph, tile_key(xx, yy));
          if (n != nullptr) {
            if (x != 0 && y != 0) {
              TileNode *nx = hashmap_get(&graph, tile_key(v->x + x, v->y + 0));
              TileNode *ny = hashmap_get(&graph, tile_key(v->x + 0, v->y + y));
              if (nx != nullptr && ny != nullptr) {
                v->neighbors[count] = n;
                count++;
              }
            } else {
              v->neighbors[count] = n;
              count++;
            }
          }
        }
      }

      v->neighbor_count = count;
    }
  }

  tm->graph = graph;
}

static float tile_distance(TileNode *lhs, TileNode *rhs) {
  float dx = lhs->x - rhs->x;
  float dy = lhs->y - rhs->y;
  return sqrtf(dx * dx + dy * dy);
}

static void astar_reset(Tilemap *tm) {
  PROFILE_FUNC();

  memset(tm->frontier.data, 0, sizeof(TileNode *) * tm->frontier.capacity);
  tm->frontier.len = 0;

  for (auto [k, v] : tm->graph) {
    v->prev = nullptr;
    v->f = 0;
    v->g = 0;
    v->h = 0;
    v->flags = 0;
  }
}

TileNode *tilemap_astar(Tilemap *tm, TilePoint start, TilePoint goal) {
  PROFILE_FUNC();

  astar_reset(tm);

  TileNode *end = hashmap_get(&tm->graph, tile_key(goal.x, goal.y));
  if (end == nullptr) {
    return nullptr;
  }

  {
    TileNode *begin = hashmap_get(&tm->graph, tile_key(start.x, start.y));
    if (begin == nullptr) {
      return nullptr;
    }

    begin->g = 0;
    begin->h = tile_distance(begin, end);
    begin->f = begin->g + begin->h;
    begin->flags |= TileNodeFlags_Open;
    priority_queue_push(&tm->frontier, begin, begin->f);
  }

  while (tm->frontier.len != 0) {
    TileNode *top = nullptr;
    priority_queue_pop(&tm->frontier, &top);
    top->flags |= TileNodeFlags_Closed;

    if (top == end) {
      return top;
    }

    for (i32 i = 0; i < top->neighbor_count; i++) {
      TileNode *next = top->neighbors[i];

      float g = top->g + next->cost + tile_distance(top, next);

      bool open = next->flags & TileNodeFlags_Open;
      if (!open || g < next->g) {
        next->prev = top;
        next->g = g;
        next->h = tile_distance(next, end);
        next->f = g + next->h;
        next->flags |= TileNodeFlags_Open;

        priority_queue_push(&tm->frontier, next, next->f);
      }
    }
  }

  return nullptr;
}
