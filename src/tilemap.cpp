#include "tilemap.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_world.h"
#include "deps/json.h"
#include "hash_map.h"
#include "json.h"
#include "prelude.h"
#include "strings.h"

static void read_f32_pair(float *x, float *y, json_array_t *arr) {
  json_array_element_t *el = arr->start;

  *x = (float)as_int(el->value);
  el = el->next;

  *y = (float)as_int(el->value);
  el = el->next;
}

static bool tile_from_json(TilemapTile *tile, json_object_t *value) {
  for (ObjectEl *el : value) {
    switch (hash(el->name)) {
    case "px"_hash: {
      read_f32_pair(&tile->x, &tile->y, as_array(el->value));
      break;
    }
    case "src"_hash: {
      read_f32_pair(&tile->u, &tile->v, as_array(el->value));
      break;
    }
    case "f"_hash: {
      tile->flip_bits = as_int(el->value);
      break;
    }
    default: break;
    }
  }

  return true;
}

static bool entity_from_json(TilemapEntity *entity, json_object_t *value) {
  for (ObjectEl *el : value) {
    switch (hash(el->name)) {
    case "__identifier"_hash: {
      entity->identifier = to_cstr(as_string(el->value));
      break;
    }
    case "px"_hash: {
      read_f32_pair(&entity->x, &entity->y, as_array(el->value));
      break;
    }
    default: break;
    }
  }

  return true;
}

static bool layer_from_json(TilemapLayer *layer, json_object_t *value,
                            Archive *ar, String filepath,
                            HashMap<Image> *images) {
  for (ObjectEl *el : value) {
    switch (hash(el->name)) {
    case "__identifier"_hash: {
      layer->identifier = to_cstr(as_string(el->value));
      break;
    }
    case "__cWid"_hash: {
      layer->c_width = as_int(el->value);
      break;
    }
    case "__cHei"_hash: {
      layer->c_height = as_int(el->value);
      break;
    }
    case "__gridSize"_hash: {
      layer->grid_size = (float)as_int(el->value);
      break;
    }
    case "__tilesetRelPath"_hash: {
      if (json_value_is_null(el->value)) {
        break;
      }

      StringBuilder sb = string_builder_make();
      defer(string_builder_trash(&sb));

      string_builder_swap_filename(&sb, filepath, as_string(el->value));

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

      break;
    }
    case "intGridCsv"_hash: {
      json_array_t *arr = as_array(el->value);

      Array<TilemapInt> grid = {};
      array_reserve(&grid, arr->length);

      for (ArrayEl *el : arr) {
        array_push(&grid, (TilemapInt)as_int(el->value));
      }

      layer->int_grid = grid;
      break;
    }
    case "gridTiles"_hash:
    case "autoLayerTiles"_hash: {
      if (layer->tiles.len != 0) {
        break;
      }

      json_array_t *arr = as_array(el->value);

      Array<TilemapTile> tiles = {};
      array_reserve(&tiles, arr->length);

      for (ArrayEl *el : arr) {
        TilemapTile tile = {};
        bool ok = tile_from_json(&tile, as_object(el->value));
        if (!ok) {
          return false;
        }
        array_push(&tiles, tile);
      }

      layer->tiles = tiles;
      break;
    }
    case "entityInstances"_hash: {
      json_array_t *arr = as_array(el->value);

      Array<TilemapEntity> entities = {};
      array_reserve(&entities, arr->length);

      for (ArrayEl *el : arr) {
        TilemapEntity entity = {};
        bool ok = entity_from_json(&entity, as_object(el->value));
        if (!ok) {
          return false;
        }
        array_push(&entities, entity);
      }

      layer->entities = entities;
      break;
    }
    default: break;
    }
  }

  for (TilemapTile &tile : layer->tiles) {
    tile.u0 = tile.u / layer->image.width;
    tile.v0 = tile.v / layer->image.height;
    tile.u1 = (tile.u + layer->grid_size) / layer->image.width;
    tile.v1 = (tile.v + layer->grid_size) / layer->image.height;
  }

  return true;
}

static bool level_from_json(TilemapLevel *level, json_object_t *value,
                            Archive *ar, String filepath,
                            HashMap<Image> *images) {
  for (ObjectEl *el : value) {
    switch (hash(el->name)) {
    case "identifier"_hash: {
      level->identifier = to_cstr(as_string(el->value));
      break;
    }
    case "iid"_hash: {
      level->iid = to_cstr(as_string(el->value));
      break;
    }
    case "worldX"_hash: {
      level->world_x = as_int(el->value);
      break;
    }
    case "worldY"_hash: {
      level->world_y = as_int(el->value);
      break;
    }
    case "pxWid"_hash: {
      level->px_width = as_int(el->value);
      break;
    }
    case "pxHei"_hash: {
      level->px_height = as_int(el->value);
      break;
    }
    case "layerInstances"_hash: {
      json_array_t *arr = as_array(el->value);

      Array<TilemapLayer> layers = {};
      array_reserve(&layers, arr->length);

      for (ArrayEl *el : arr) {
        TilemapLayer layer = {};
        bool ok =
            layer_from_json(&layer, as_object(el->value), ar, filepath, images);
        if (!ok) {
          return false;
        }
        array_push(&layers, layer);
      }

      level->layers = layers;
      break;
    }
    default: break;
    }
  }

  return true;
}

static void *json_parse_alloc(void *, size_t bytes) { return mem_alloc(bytes); }

bool tilemap_load(Tilemap *tm, Archive *ar, String filepath) {
  String contents = {};
  bool ok = ar->read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  json_value_t *json =
      json_parse_ex(contents.data, contents.len, json_parse_flags_default,
                    json_parse_alloc, nullptr, nullptr);
  defer(mem_free(json));

  HashMap<Image> images = {};

  Tilemap tilemap = {};

  for (ObjectEl *el : as_object(json)) {
    switch (hash(el->name)) {
    case "levels"_hash: {
      json_array_t *arr = as_array(el->value);

      Array<TilemapLevel> levels = {};
      array_reserve(&levels, arr->length);

      for (ArrayEl *el : arr) {
        TilemapLevel level = {};
        bool ok = level_from_json(&level, as_object(el->value), ar, filepath,
                                  &images);
        if (!ok) {
          return false;
        }
        array_push(&levels, level);
      }

      tilemap.levels = levels;
      break;
    }
    default: break;
    }
  }

  tilemap.images = images;

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
