#include "tilemap.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_world.h"
#include "hash_map.h"
#include "json.h"
#include "prelude.h"
#include "profile.h"
#include "strings.h"

static bool layer_from_json(TilemapLayer *layer, JSON *json, Archive *ar,
                            String filepath, HashMap<Image> *images) {
  PROFILE_FUNC();

  layer->identifier = to_cstr(json_lookup_string(json, "__identifier"));
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

  JSONArray *arr_tiles = (grid_tiles != nullptr && grid_tiles->count != 0)
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

  Array<TilemapInt> grid = {};
  if (int_grid_csv != nullptr) {
    PROFILE_BLOCK("int grid");

    array_reserve(&grid, int_grid_csv->count);
    for (JSONArray *a = int_grid_csv; a != nullptr; a = a->next) {
      array_push(&grid, (TilemapInt)json_number(&a->value));
    }
  }
  layer->int_grid = grid;

  Array<TilemapTile> tiles = {};
  if (arr_tiles != nullptr) {
    PROFILE_BLOCK("tiles");

    array_reserve(&tiles, arr_tiles->count);
    for (JSONArray *a = arr_tiles; a != nullptr; a = a->next) {
      JSON *px = json_lookup(&a->value, "px");
      JSON *src = json_lookup(&a->value, "src");

      TilemapTile tile = {};
      tile.x = json_number(json_index(px, 0));
      tile.y = json_number(json_index(px, 1));

      tile.u = json_number(json_index(src, 0));
      tile.v = json_number(json_index(src, 1));

      tile.flip_bits = (i32)json_lookup_number(&a->value, "f");
      array_push(&tiles, tile);
    }
  }
  layer->tiles = tiles;

  for (TilemapTile &tile : layer->tiles) {
    tile.u0 = tile.u / layer->image.width;
    tile.v0 = tile.v / layer->image.height;
    tile.u1 = (tile.u + layer->grid_size) / layer->image.width;
    tile.v1 = (tile.v + layer->grid_size) / layer->image.height;
  }

  Array<TilemapEntity> entities = {};
  if (entity_instances != nullptr) {
    PROFILE_BLOCK("entities");

    array_reserve(&entities, entity_instances->count);
    for (JSONArray *a = entity_instances; a != nullptr; a = a->next) {
      JSON *px = json_lookup(&a->value, "px");

      TilemapEntity entity = {};
      entity.x = json_number(json_index(px, 0));
      entity.y = json_number(json_index(px, 1));
      entity.identifier =
          to_cstr(json_lookup_string(&a->value, "__identifier"));

      array_push(&entities, entity);
    }
  }
  layer->entities = entities;

  return true;
}

static bool level_from_json(TilemapLevel *level, JSON *json, Archive *ar,
                            String filepath, HashMap<Image> *images) {
  PROFILE_FUNC();

  level->identifier = to_cstr(json_lookup_string(json, "identifier"));
  level->iid = to_cstr(json_lookup_string(json, "iid"));
  level->world_x = json_lookup_number(json, "worldX");
  level->world_y = json_lookup_number(json, "worldY");
  level->px_width = json_lookup_number(json, "pxWid");
  level->px_height = json_lookup_number(json, "pxHei");

  JSONArray *layer_instances = json_array(json_lookup(json, "layerInstances"));

  Array<TilemapLayer> layers = {};
  if (layer_instances != nullptr) {
    array_reserve(&layers, layer_instances->count);
    for (JSONArray *a = layer_instances; a != nullptr; a = a->next) {
      TilemapLayer layer = {};
      bool ok = layer_from_json(&layer, &a->value, ar, filepath, images);
      if (!ok) {
        return false;
      }
      array_push(&layers, layer);
    }
  }
  level->layers = layers;

  return true;
}

bool tilemap_load(Tilemap *tm, Archive *ar, String filepath) {
  PROFILE_FUNC();

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

  Array<TilemapLevel> levels = {};
  if (arr_levels != nullptr) {
    array_reserve(&levels, arr_levels->count);
    for (JSONArray *a = arr_levels; a != nullptr; a = a->next) {
      TilemapLevel level = {};
      bool ok = level_from_json(&level, &a->value, ar, filepath, &images);
      if (!ok) {
        return false;
      }
      array_push(&levels, level);
    }
  }

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
  for (TilemapLevel &level : tm->levels) {
    for (TilemapLayer &layer : level.layers) {
      for (TilemapEntity &entity : layer.entities) {
        mem_free(entity.identifier.data);
      }

      mem_free(layer.identifier.data);
      array_trash(&layer.entities);
      array_trash(&layer.tiles);
      array_trash(&layer.int_grid);
    }

    mem_free(level.identifier.data);
    mem_free(level.iid.data);
    array_trash(&level.layers);
  }

  for (auto [k, v] : tm->images) {
    image_trash(v);
  }

  array_trash(&tm->levels);
  hashmap_trash(&tm->images);
  hashmap_trash(&tm->bodies);
}

static void make_collision_for_layer(b2Body *body, TilemapLayer *layer,
                                     float world_x, float world_y, float meter,
                                     Array<TilemapInt> *walls) {
  PROFILE_FUNC();

  auto is_wall = [layer, walls](i32 y, i32 x) {
    if (x >= layer->c_width || y >= layer->c_height) {
      return false;
    }

    for (TilemapInt n : *walls) {
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
                            String layer_name, Array<TilemapInt> *walls) {
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
