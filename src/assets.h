#pragma once

#include "deps/lua/lua.h"
#include "image.h"
#include "sprite.h"
#include "tilemap.h"

enum AssetKind : u64 {
  AssetKind_None,
  AssetKind_LuaRef,
  AssetKind_Image,
  AssetKind_Sprite,
  AssetKind_Tilemap,
};

struct Asset {
  char *name;
  u64 hash;
  u64 modtime;
  AssetKind kind;
  union {
    i32 lua_ref;
    Image image;
    SpriteData sprite;
    Tilemap tilemap;
  };
};

struct Assets {
  HashMap<Asset> table;
  cute_rw_lock_t rw_lock;
};

void assets_make(Assets *assets);
void assets_trash(Assets *assets);

struct AssetLoad {
  Asset *data;
  bool found;
};

AssetLoad asset_load(Assets *assets, AssetKind kind, String filepath);
void asset_load_unlock(Assets *assets, AssetLoad *load);

const Asset *check_asset(lua_State *L, Assets *assets, u64 key);
const Asset *check_asset_mt(lua_State *L, Assets *assets, const char *mt);
Asset *write_asset_mt(lua_State *L, Assets *assets, const char *mt);
