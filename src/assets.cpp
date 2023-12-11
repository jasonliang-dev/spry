#include "assets.h"
#include "app.h"
#include "luax.h"
#include "os.h"
#include "profile.h"
#include "strings.h"
#include "sync.h"

struct FileChange {
  u64 key;
  u64 modtime;
};

struct Assets {
  HashMap<Asset> table;
  RWLock rw_lock;

  Mutex mtx;
  Cond notify;
  bool shutdown;

  Thread reload_thread;
  Array<FileChange> changes;
};

static Assets g_assets = {};

static void hot_reload_thread(void *) {
  u32 reload_interval = g_app->reload_interval.load();

  while (true) {
    PROFILE_BLOCK("hot reload");

    if (LockGuard lock{&g_assets.mtx}) {
      if (g_assets.shutdown) {
        return;
      }

      bool signaled =
          g_assets.notify.timed_wait(&g_assets.mtx, reload_interval);
      if (signaled) {
        return;
      }
    }

    {
      PROFILE_BLOCK("check for updates");

      g_assets.rw_lock.shared_lock();
      defer(g_assets.rw_lock.shared_unlock());

      g_assets.changes.len = 0;
      for (auto [k, v] : g_assets.table) {
        PROFILE_BLOCK("read modtime");

        u64 modtime = os_file_modtime(v->name.data);
        if (modtime > v->modtime) {
          FileChange change = {};
          change.key = v->hash;
          change.modtime = modtime;
          g_assets.changes.push(change);
        }
      }
    }

    if (g_assets.changes.len > 0) {
      PROFILE_BLOCK("perform hot reload");
      LockGuard lock(&g_app->frame_mtx);

      for (FileChange change : g_assets.changes) {
        Asset a = {};
        bool exists = asset_read(change.key, &a);
        assert(exists);

        a.modtime = change.modtime;

        bool ok = false;
        switch (a.kind) {
        case AssetKind_LuaRef: {
          luaL_unref(g_app->L, LUA_REGISTRYINDEX, a.lua_ref);
          a.lua_ref = luax_require_script(g_app->L, a.name);
          ok = true;
          break;
        }
        case AssetKind_Image: {
          a.image.trash();
          ok = a.image.load(a.name);
          break;
        }
        case AssetKind_Sprite: {
          a.sprite.trash();
          ok = a.sprite.load(a.name);
          break;
        }
        case AssetKind_Tilemap: {
          a.tilemap.trash();
          ok = a.tilemap.load(a.name);
          break;
        }
        default: continue; break;
        }

        if (!ok) {
          fatal_error(tmp_fmt("failed to hot reload: %s", a.name.data));
          return;
        }

        asset_write(a);
        printf("reloaded: %s\n", a.name.data);
      }
    }
  }
}

void assets_shutdown() {
  if (g_app->hot_reload_enabled.load()) {
    if (LockGuard lock{&g_assets.mtx}) {
      g_assets.shutdown = true;
    }

    g_assets.notify.signal();
    g_assets.reload_thread.join();
    g_assets.changes.trash();
  }

  for (auto [k, v] : g_assets.table) {
    mem_free(v->name.data);

    switch (v->kind) {
    case AssetKind_Image: v->image.trash(); break;
    case AssetKind_Sprite: v->sprite.trash(); break;
    case AssetKind_Tilemap: v->tilemap.trash(); break;
    default: break;
    }
  }
  g_assets.table.trash();
}

void assets_start_hot_reload() {
  if (g_app->hot_reload_enabled.load()) {
    g_assets.reload_thread.make(hot_reload_thread, nullptr);
  }
}

bool asset_load(AssetKind kind, String filepath, Asset *out) {
  PROFILE_FUNC();

  u64 key = fnv1a(filepath);

  {
    Asset asset = {};
    if (asset_read(key, &asset)) {
      if (out != nullptr) {
        *out = asset;
      }
      return true;
    }
  }

  {
    PROFILE_BLOCK("load new asset");

    Asset asset = {};
    asset.name = to_cstr(filepath);
    asset.hash = key;
    {
      PROFILE_BLOCK("asset modtime")
      asset.modtime = os_file_modtime(asset.name.data);
    }
    asset.kind = kind;

    bool ok = false;
    switch (kind) {
    case AssetKind_LuaRef: {
      asset.lua_ref = LUA_REFNIL;
      asset_write(asset);
      asset.lua_ref = luax_require_script(g_app->L, filepath);
      ok = true;
      break;
    }
    case AssetKind_Image: ok = asset.image.load(filepath); break;
    case AssetKind_Sprite: ok = asset.sprite.load(filepath); break;
    case AssetKind_Tilemap: ok = asset.tilemap.load(filepath); break;
    default: break;
    }

    if (!ok) {
      mem_free(asset.name.data);
      return false;
    }

    asset_write(asset);

    if (out != nullptr) {
      *out = asset;
    }
    return true;
  }
}

bool asset_read(u64 key, Asset *out) {
  g_assets.rw_lock.shared_lock();
  defer(g_assets.rw_lock.shared_unlock());

  const Asset *asset = g_assets.table.get(key);
  if (asset == nullptr) {
    return false;
  }

  *out = *asset;
  return true;
}

void asset_write(Asset asset) {
  g_assets.rw_lock.unique_lock();
  defer(g_assets.rw_lock.unique_unlock());

  g_assets.table[asset.hash] = asset;
}

Asset check_asset(lua_State *L, u64 key) {
  Asset asset = {};
  if (!asset_read(key, &asset)) {
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
