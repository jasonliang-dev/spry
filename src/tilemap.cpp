#include "tilemap.h"
#include "deps/json.h"
#include "hash_map.h"
#include "json.h"
#include "prelude.h"
#include "strings.h"

static void float_pair(float *x, float *y, json_array_t *arr) {
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
      float_pair(&tile->x, &tile->y, as_array(el->value));
      break;
    }
    case "src"_hash: {
      float_pair(&tile->u, &tile->v, as_array(el->value));
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
      entity->identifier = clone(as_string(el->value));
      break;
    }
    case "px"_hash: {
      float_pair(&entity->x, &entity->y, as_array(el->value));
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
      layer->identifier = clone(as_string(el->value));
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
      defer(drop(&sb));

      relative_path(&sb, filepath, as_string(el->value));

      String fullpath = as_string(&sb);
      u64 key = fnv1a(fullpath);

      Image *img = get(images, key);
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
      reserve(&grid, arr->length);

      for (ArrayEl *el : arr) {
        push(&grid, (TilemapInt)as_int(el->value));
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
      reserve(&tiles, arr->length);

      for (ArrayEl *el : arr) {
        TilemapTile tile = {};
        bool ok = tile_from_json(&tile, as_object(el->value));
        if (!ok) {
          return false;
        }
        push(&tiles, tile);
      }

      layer->tiles = tiles;
      break;
    }
    case "entityInstances"_hash: {
      json_array_t *arr = as_array(el->value);

      Array<TilemapEntity> entities = {};
      reserve(&entities, arr->length);

      for (ArrayEl *el : arr) {
        TilemapEntity entity = {};
        bool ok = entity_from_json(&entity, as_object(el->value));
        if (!ok) {
          return false;
        }
        push(&entities, entity);
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

static bool neighbor_from_json(TilemapNeighbor *neighbor, json_object_t *value,
                               Array<TilemapLayer> *layers) {
  for (ObjectEl *el : value) {
    switch (hash(el->name)) {
    case "levelIid"_hash: {
      neighbor->iid = clone(as_string(el->value));
      break;
    }
    case "dir"_hash: {
      TilemapDir dir = {};
      String str = as_string(el->value);
      if (str.len == 1) {
        switch (str.data[0]) {
        case 'n': dir = TilemapDir_North; break;
        case 'e': dir = TilemapDir_East; break;
        case 's': dir = TilemapDir_South; break;
        case 'w': dir = TilemapDir_West; break;
        }
      }
      neighbor->dir = dir;
      break;
    }
    default: break;
    }
  }
  return true;
}

static bool level_from_json(TilemapLevel *level, json_object_t *value,
                            Archive *ar, String filepath,
                            HashMap<Image> *images) {
  for (ObjectEl *el : value) {
    switch (hash(el->name)) {
    case "identifier"_hash: {
      level->identifier = clone(as_string(el->value));
      break;
    }
    case "iid"_hash: {
      level->iid = clone(as_string(el->value));
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
      reserve(&layers, arr->length);

      for (ArrayEl *el : arr) {
        TilemapLayer layer = {};
        bool ok =
            layer_from_json(&layer, as_object(el->value), ar, filepath, images);
        if (!ok) {
          return false;
        }
        push(&layers, layer);
      }

      level->layers = layers;
      break;
    }
    case "__neighbours"_hash: {
      json_array_t *arr = as_array(el->value);

      Array<TilemapNeighbor> neighbors = {};
      reserve(&neighbors, arr->length);

      for (ArrayEl *el : arr) {
        TilemapNeighbor neighbor;
        bool ok =
            neighbor_from_json(&neighbor, as_object(el->value), &level->layers);
        if (!ok) {
          return false;
        }
        push(&neighbors, neighbor);
      }

      level->neighbors = neighbors;
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
  bool ok = ar->read_entire_file(ar, &contents, filepath);
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
      reserve(&levels, arr->length);

      for (ArrayEl *el : arr) {
        TilemapLevel level = {};
        bool ok = level_from_json(&level, as_object(el->value), ar, filepath,
                                  &images);
        if (!ok) {
          return false;
        }
        push(&levels, level);
      }

      for (TilemapLevel &level : levels) {
        for (TilemapNeighbor &neighbour : level.neighbors) {
          for (TilemapLevel &level : levels) {
            if (neighbour.iid == level.iid) {
              neighbour.level = &level;
            }
          }
        }
      }

      tilemap.levels = levels;
      break;
    }
    default: break;
    }
  }

  HashMap<TilemapGrid> grids = {};

  for (TilemapLevel &level : tilemap.levels) {
    for (TilemapLayer &layer : level.layers) {
      if (layer.int_grid.len > 0) {
        u64 key = fnv1a(layer.identifier);
        TilemapGrid &grid = grids[key];
        grid.count += layer.int_grid.len;
        grid.grid_size = layer.grid_size;
      }
    }
  }

  for (TilemapLevel &level : tilemap.levels) {
    for (TilemapLayer &layer : level.layers) {
      if (layer.int_grid.len == 0) {
        continue;
      }

      u64 key = fnv1a(layer.identifier);
      TilemapGrid *grid = get(&grids, key);
      assert(grid != nullptr);
      reserve(&grid->values, hash_map_reserve_size(grid->count));

      for (i32 y = 0; y < layer.c_height; y++) {
        for (i32 x = 0; x < layer.c_width; x++) {
          TilemapPoint point;
          point.x = x + (i32)(level.world_x / layer.grid_size);
          point.y = y + (i32)(level.world_y / layer.grid_size);

          TilemapInt value = layer.int_grid[y * layer.c_width + x];
          grid->values[point.value] = value;
        }
      }
    }
  }

  tilemap.images = images;
  tilemap.grids_by_layer = grids;

  printf("loaded tilemap with %llu levels\n", tilemap.levels.len);
  *tm = tilemap;
  return true;
}

void drop(Tilemap *tm) {
  for (TilemapLevel &level : tm->levels) {
    for (TilemapLayer &layer : level.layers) {
      for (TilemapEntity &entity : layer.entities) {
        mem_free(entity.identifier.data);
      }

      mem_free(layer.identifier.data);
      drop(&layer.entities);
      drop(&layer.tiles);
      drop(&layer.int_grid);
    }

    for (TilemapNeighbor &neighbor : level.neighbors) {
      mem_free(neighbor.iid.data);
    }

    mem_free(level.identifier.data);
    mem_free(level.iid.data);
    drop(&level.layers);
    drop(&level.neighbors);
  }

  for (auto [k, v] : tm->images) {
    drop(v);
  }

  for (auto [k ,v] : tm->grids_by_layer) {
    drop(&v->values);
  }

  drop(&tm->levels);
  drop(&tm->images);
  drop(&tm->grids_by_layer);
}

#if 0
static TilemapLevel *find_neighbor(TilemapLevel *node, float x, float y) {
  node->visited = true;

  if (node->world_x <= x && x < node->world_x + node->px_width &&
      node->world_y <= y && y < node->world_y + node->px_height) {
    return node;
  }

  for (TilemapNeighbor &neighbor : node->neighbors) {
    TilemapLevel *level = neighbor.level;
    if (level->visited) {
      continue;
    }

    TilemapLevel *find_result = find_neighbor(level, x, y);
    if (find_result != nullptr) {
      return find_result;
    }
  }

  return nullptr;
}
#endif

void tilemap_grid_begin(Tilemap *tm, String layer) {
  u64 key = fnv1a(layer);
  TilemapGrid *grid = get(&tm->grids_by_layer, key);
  tm->current_grid = grid;
}

void tilemap_grid_end(Tilemap *tm) { tm->current_grid = nullptr; }

TilemapInt tilemap_grid_value(Tilemap *tm, float x, float y) {
  TilemapGrid *grid = tm->current_grid;
  if (grid == nullptr) {
    return -1;
  }

  TilemapPoint point = {};
  point.x = (i32)(x / grid->grid_size);
  point.y = (i32)(y / grid->grid_size);

  TilemapInt *i = get(&grid->values, point.value);
  if (i == nullptr) {
    return -1;
  }

  return *i;
}

bool tilemap_rect_every(Tilemap *tm, TilemapInt needle, float x0, float y0,
                        float x1, float y1) {
  TilemapGrid *grid = tm->current_grid;
  if (grid == nullptr) {
    return false;
  }

  i32 left = (i32)(x0 / grid->grid_size);
  i32 top = (i32)(y0 / grid->grid_size);

  i32 right = (i32)(x1 / grid->grid_size);
  i32 bot = (i32)(y1 / grid->grid_size);

  for (i32 x = left; x <= right; x++) {
    for (i32 y = top; y <= bot; y++) {
      TilemapPoint point = {};
      point.x = x;
      point.y = y;
      TilemapInt *i = get(&grid->values, point.value);
      TilemapInt cmp = i == nullptr ? -1 : *i;
      if (needle != cmp) {
        return false;
      }
    }
  }

  return true;
}

bool tilemap_rect_has(Tilemap *tm, TilemapInt needle, float x0, float y0,
                      float x1, float y1) {
  TilemapGrid *grid = tm->current_grid;
  if (grid == nullptr) {
    return false;
  }

  i32 left = (i32)(x0 / grid->grid_size);
  i32 top = (i32)(y0 / grid->grid_size);

  i32 right = (i32)(x1 / grid->grid_size);
  i32 bot = (i32)(y1 / grid->grid_size);

  for (i32 x = left; x <= right; x++) {
    for (i32 y = top; y <= bot; y++) {
      TilemapPoint point = {};
      point.x = x;
      point.y = y;
      TilemapInt *i = get(&grid->values, point.value);
      TilemapInt cmp = i == nullptr ? -1 : *i;
      if (needle == cmp) {
        return true;
      }
    }
  }

  return false;
}
