#include "assets.h"
#include "app.h"
#include "deps/cute_sync.h"
#include "deps/lua/lauxlib.h"
#include "luax.h"
#include "os.h"
#include "profile.h"

struct FileChange {
  u64 key;
  u64 modtime;
};

struct Assets {
  HashMap<Asset> table;
  cute_rw_lock_t rw_lock;

  cute_atomic_int_t shutdown_request;
  cute_thread_t *reload_thread;
  Array<FileChange> changes;
};

static Assets g_assets;

static i32 hot_reload_thread(void *) {
  if (cute_atomic_get(&g_app->hot_reload_enabled) == 0) {
    return 0;
  }

  u32 reload_interval = cute_atomic_get(&g_app->reload_interval);
  while (cute_atomic_get(&g_assets.shutdown_request) == 0) {
    PROFILE_BLOCK("hot reload");

    os_sleep(reload_interval);

    g_assets.changes.len = 0;

    {
      PROFILE_BLOCK("check for updates");

      cute_read_lock(&g_assets.rw_lock);
      defer(cute_read_unlock(&g_assets.rw_lock));

      for (auto [k, v] : g_assets.table) {
        PROFILE_BLOCK("reload file asset");

        u64 modtime = os_file_modtime(v->name);
        if (modtime > v->modtime) {
          FileChange change = {};
          change.key = v->hash;
          change.modtime = modtime;
          array_push(&g_assets.changes, change);
        }
      }
    }

    if (g_assets.changes.len > 0) {
      PROFILE_BLOCK("perform hot reload");

      cute_lock(&g_app->frame_mtx);
      defer(cute_unlock(&g_app->frame_mtx));

      cute_write_lock(&g_assets.rw_lock);
      defer(cute_write_unlock(&g_assets.rw_lock));

      for (FileChange change : g_assets.changes) {
        Asset *a = nullptr;
        bool exists = hashmap_index(&g_assets.table, change.key, &a);
        assert(exists);

        a->modtime = change.modtime;

        bool skip = false;
        bool ok = false;
        switch (a->kind) {
        case AssetKind_LuaRef: {
          luaL_unref(g_app->L, LUA_REGISTRYINDEX, a->lua_ref);
          a->lua_ref = require_lua_script(g_app->L, g_app->archive, a->name);
          ok = true;
          break;
        }
        case AssetKind_Image: {
          image_trash(&a->image);
          ok = image_load(&a->image, g_app->archive, a->name);
          break;
        }
        case AssetKind_Sprite: {
          sprite_data_trash(&a->sprite);
          ok = sprite_data_load(&a->sprite, g_app->archive, a->name);
          break;
        }
        case AssetKind_Tilemap: {
          tilemap_trash(&a->tilemap);
          ok = tilemap_load(&a->tilemap, g_app->archive, a->name);
          break;
        }
        default: skip = true; break;
        }

        if (!skip) {
          if (!ok) {
            fatal_error(tmp_fmt("failed to hot reload: %s", a->name));
          } else {
            printf("reloaded: %s\n", a->name);
          }
        }
      }
    }
  }

  return 0;
}

void assets_setup() {
  g_assets = {};
  g_assets.rw_lock = cute_rw_lock_create();
}

void assets_shutdown() {
  {
    PROFILE_BLOCK("wait for hot reload");

    cute_atomic_set(&g_assets.shutdown_request, 1);
    cute_thread_wait(g_assets.reload_thread);
  }
  array_trash(&g_assets.changes);

  cute_rw_lock_destroy(&g_assets.rw_lock);

  for (auto [k, v] : g_assets.table) {
    mem_free(v->name);

    switch (v->kind) {
    case AssetKind_Image: image_trash(&v->image); break;
    case AssetKind_Sprite: sprite_data_trash(&v->sprite); break;
    case AssetKind_Tilemap: tilemap_trash(&v->tilemap); break;
    default: break;
    }
  }
  hashmap_trash(&g_assets.table);
}

void assets_start_hot_reload() {
  g_assets.reload_thread =
      cute_thread_create(hot_reload_thread, "hot reload", nullptr);
}

bool asset_load(AssetKind kind, Archive *ar, String filepath, Asset *out) {
  PROFILE_FUNC();

  u64 key = fnv1a(filepath);

  {
    cute_read_lock(&g_assets.rw_lock);
    defer(cute_read_unlock(&g_assets.rw_lock));

    Asset *asset = hashmap_get(&g_assets.table, key);
    if (asset != nullptr) {
      assert(asset->kind == kind);

      if (out != nullptr) {
        *out = *asset;
      }
      return true;
    }
  }

  {
    PROFILE_BLOCK("load new asset");

    cute_write_lock(&g_assets.rw_lock);
    defer(cute_write_unlock(&g_assets.rw_lock));

    Asset *asset = nullptr;
    hashmap_index(&g_assets.table, key, &asset);
    asset->name = to_cstr(filepath).data;
    asset->hash = key;
    asset->modtime = os_file_modtime(asset->name);
    asset->kind = kind;

    bool ok = false;
    switch (kind) {
    case AssetKind_LuaRef: {
      asset->lua_ref = require_lua_script(g_app->L, ar, filepath);
      ok = true;
      break;
    }
    case AssetKind_Image: {
      ok = image_load(&asset->image, ar, filepath);
      break;
    }
    case AssetKind_Sprite: {
      ok = sprite_data_load(&asset->sprite, ar, filepath);
      break;
    }
    case AssetKind_Tilemap: {
      ok = tilemap_load(&asset->tilemap, ar, filepath);
      break;
    }
    default: break;
    }

    if (!ok) {
      hashmap_unset(&g_assets.table, key);
      return false;
    }

    if (out != nullptr) {
      *out = *asset;
    }
    return true;
  }
}

bool asset_read(u64 key, Asset *out) {
  cute_read_lock(&g_assets.rw_lock);
  defer(cute_read_unlock(&g_assets.rw_lock));

  const Asset *asset = hashmap_get(&g_assets.table, key);
  if (asset == nullptr) {
    return false;
  }

  *out = *asset;
  return true;
}

void asset_write(Asset asset) {
  cute_write_lock(&g_assets.rw_lock);
  defer(cute_write_unlock(&g_assets.rw_lock));

  Asset *dst = hashmap_get(&g_assets.table, asset.hash);
  *dst = asset;
}

Asset check_asset(lua_State *L, u64 key) {
  Asset asset = {};
  bool ok = asset_read(key, &asset);
  if (!ok) {
    luaL_error(L, "cannot read asset");
  }

  return asset;
}

Asset check_asset_mt(lua_State *L, i32 arg, const char *mt) {
  u64 *udata = (u64 *)luaL_checkudata(L, arg, mt);

  Asset asset = {};
  bool ok = asset_read(*udata, &asset);
  if (!ok) {
    luaL_error(L, "cannot read asset");
  }

  return asset;
}
