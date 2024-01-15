#pragma once

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

struct AssetLoadData {
  AssetKind kind;
  bool generate_mips;
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

void assets_shutdown();
void assets_start_hot_reload();
void assets_perform_hot_reload_changes();

bool asset_load_kind(AssetKind kind, String filepath, Asset *out);
bool asset_load(AssetLoadData desc, String filepath, Asset *out);

bool asset_read(u64 key, Asset *out);
void asset_write(Asset asset);

struct lua_State;
Asset check_asset(lua_State *L, u64 key);
Asset check_asset_mt(lua_State *L, i32 arg, const char *mt);
