#pragma once

#include "sync.h"
#include "deps/lua/lua.h"
#include "image.h"
#include "sprite.h"
#include "tilemap.h"

enum AssetKind : i32 {
  AssetKind_None,
  AssetKind_LuaRef,
  AssetKind_Image,
  AssetKind_Sprite,
  AssetKind_Tilemap,
};

struct Asset {
  String name;
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

void assets_setup();
void assets_shutdown();
void assets_start_hot_reload();

bool asset_load(AssetKind kind, String filepath, Asset *out);

bool asset_read(u64 key, Asset *out);
void asset_write(Asset asset);

Asset check_asset(lua_State *L, u64 key);
Asset check_asset_mt(lua_State *L, i32 arg, const char *mt);
