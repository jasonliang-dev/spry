#include "assets.h"
#include "deps/lua/lauxlib.h"
#include "os.h"

void assets_make(Assets *assets) {
  *assets = {};
  assets->rw_lock = cute_rw_lock_create();
}

void assets_trash(Assets *assets) {
  cute_rw_lock_destroy(&assets->rw_lock);

  for (auto [k, v] : assets->table) {
    mem_free(v->name);

    switch (v->kind) {
    case AssetKind_Image: image_trash(&v->image); break;
    case AssetKind_Sprite: sprite_data_trash(&v->sprite); break;
    case AssetKind_Tilemap: tilemap_trash(&v->tilemap); break;
    case AssetKind_None:
    default: break;
    }
  }
  hashmap_trash(&assets->table);
}

AssetLoad asset_load(Assets *assets, AssetKind kind, String filepath) {
  AssetLoad out = {};

  Asset *asset = nullptr;
  u64 key = fnv1a(filepath);

  cute_read_lock(&assets->rw_lock);

  asset = hashmap_get(&assets->table, key);
  if (asset != nullptr) {
    assert(asset->kind == kind);
    out.data = asset;
    out.found = true;
    return out;
  }

  cute_read_unlock(&assets->rw_lock);
  cute_write_lock(&assets->rw_lock);

  hashmap_index(&assets->table, key, &asset);
  asset->name = to_cstr(filepath).data;
  asset->hash = key;
  asset->modtime = os_file_modtime(asset->name);
  asset->kind = kind;

  out.data = asset;
  out.found = false;
  return out;
}

void asset_load_unlock(Assets *assets, AssetLoad *load) {
  if (load->found) {
    cute_read_unlock(&assets->rw_lock);
  } else {
    cute_write_unlock(&assets->rw_lock);
  }
}

const Asset *check_asset(lua_State *L, Assets *assets, u64 key) {
  cute_read_lock(&assets->rw_lock);
  const Asset *a = hashmap_get(&assets->table, key);

  if (a == nullptr) {
    cute_read_unlock(&assets->rw_lock);
    luaL_error(L, "cannot read asset");
  }

  return a;
}

const Asset *check_asset_mt(lua_State *L, Assets *assets, const char *mt) {
  u64 *udata = (u64 *)luaL_checkudata(L, 1, mt);

  cute_read_lock(&assets->rw_lock);
  const Asset *a = hashmap_get(&assets->table, *udata);

  if (a == nullptr) {
    cute_read_unlock(&assets->rw_lock);
    luaL_error(L, "cannot read asset from %s", mt);
  }

  return a;
}

Asset *write_asset_mt(lua_State *L, Assets *assets, const char *mt) {
  u64 *udata = (u64 *)luaL_checkudata(L, 1, mt);

  cute_write_lock(&assets->rw_lock);
  Asset *a = hashmap_get(&assets->table, *udata);

  if (a == nullptr) {
    cute_write_unlock(&assets->rw_lock);
    luaL_error(L, "no write access to asset from %s", mt);
  }

  return a;
}
