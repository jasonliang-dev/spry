#include "api.h"
#include "app.h"
#include "archive.h"
#include "array.h"
#include "assets.h"
#include "deps/cute_sync.h"
#include "deps/lua/lauxlib.h"
#include "deps/lua/lua.h"
#include "deps/lua/lualib.h"
#include "deps/sokol_app.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "deps/sokol_glue.h"
#include "deps/sokol_log.h"
#include "deps/sokol_time.h"
#include "draw.h"
#include "font.h"
#include "image.h"
#include "luax.h"
#include "os.h"
#include "prelude.h"
#include "profile.h"
#include "sprite.h"
#include "strings.h"
#include "tilemap.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

FORMAT_ARGS(1)
static void panic(const char *fmt, ...) {
  va_list args = {};
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");

  exit(1);
}

static void *sokol_alloc(size_t size, void *) {
  size_t bytes = size;
  return mem_alloc(bytes);
}

static void sokol_free(void *ptr, void *) {
  void *mem = ptr;
  mem_free(mem);
}

static void init() {
  PROFILE_FUNC();

  {
    PROFILE_BLOCK("sokol");

    sg_desc sg = {};
    sg.logger.func = slog_func;
    sg.context = sapp_sgcontext();
    sg.allocator.alloc_fn = sokol_alloc;
    sg.allocator.free_fn = sokol_free;
    sg_setup(sg);

    sgl_desc_t sgl = {};
    sgl.logger.func = slog_func;
    sgl.allocator.alloc_fn = sokol_alloc;
    sgl.allocator.free_fn = sokol_free;
    sgl_setup(sgl);

    sg_pipeline_desc sg_pipline = {};
    sg_pipline.depth.write_enabled = true;
    sg_pipline.colors[0].blend.enabled = true;
    sg_pipline.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    sg_pipline.colors[0].blend.dst_factor_rgb =
        SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    g_app->pipeline = sgl_make_pipeline(sg_pipline);
  }

  {
    PROFILE_BLOCK("miniaudio");

    ma_engine_config ma_config = {};
    ma_config.channels = 2;
    ma_config.sampleRate = 44100;
    ma_result res = ma_engine_init(&ma_config, &g_app->audio_engine);
    if (res != MA_SUCCESS) {
      fatal_error("failed to initialize audio engine");
    }
  }

  renderer_setup(&g_app->renderer);

  g_app->time.last = stm_now();

  {
    PROFILE_BLOCK("spry.start");

    lua_State *L = g_app->L;

    if (!g_app->error_mode) {
      lua_getglobal(L, "spry");
      lua_getfield(L, -1, "start");
      lua_remove(L, -2);
      if (lua_pcall(L, 0, 0, 1) != LUA_OK) {
        lua_pop(L, 1);
      }
    }
  }

  g_app->frame_mtx = cute_mutex_create();

  assets_start_hot_reload();

#ifdef DEBUG
  printf("end of init\n");
#endif
}

static void event(const sapp_event *e) {
  switch (e->type) {
  case SAPP_EVENTTYPE_KEY_DOWN: g_app->key_state[e->key_code] = true; break;
  case SAPP_EVENTTYPE_KEY_UP: g_app->key_state[e->key_code] = false; break;
  case SAPP_EVENTTYPE_MOUSE_DOWN:
    g_app->mouse_state[e->mouse_button] = true;
    break;
  case SAPP_EVENTTYPE_MOUSE_UP:
    g_app->mouse_state[e->mouse_button] = false;
    break;
  case SAPP_EVENTTYPE_MOUSE_MOVE:
    g_app->mouse_x = e->mouse_x;
    g_app->mouse_y = e->mouse_y;
    break;
  case SAPP_EVENTTYPE_MOUSE_SCROLL:
    g_app->scroll_x = e->scroll_x;
    g_app->scroll_y = e->scroll_y;
    break;
  default: break;
  }
}

static void frame() {
  PROFILE_FUNC();

  cute_lock(&g_app->frame_mtx);
  defer(cute_unlock(&g_app->frame_mtx));

  {
    AppTime *time = &g_app->time;
    u64 lap = stm_laptime(&time->last);
    time->delta = stm_sec(lap);
    time->accumulator += lap;

#ifndef __EMSCRIPTEN__
    if (time->target_ticks > 0) {
      u64 TICK_MS = 1000000;
      u64 TICK_US = 1000;

      u64 target = time->target_ticks;

      if (time->accumulator < target) {
        u64 ms = (target - time->accumulator) / TICK_MS;
        if (ms > 0) {
          PROFILE_BLOCK("sleep");
          os_sleep(ms - 1);
        }

        {
          PROFILE_BLOCK("spin loop");

          u64 lap = stm_laptime(&time->last);
          time->delta += stm_sec(lap);
          time->accumulator += lap;

          while (time->accumulator < target) {
            os_yield();

            u64 lap = stm_laptime(&time->last);
            time->delta += stm_sec(lap);
            time->accumulator += lap;
          }
        }
      }

      u64 fuzz = TICK_US * 100;
      while (time->accumulator >= target - fuzz) {
        if (time->accumulator < target + fuzz) {
          time->accumulator = 0;
        } else {
          time->accumulator -= target + fuzz;
        }
      }
    }
#endif
  }

  {
    PROFILE_BLOCK("begin render pass");

    sg_pass_action pass = {};
    pass.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.colors[0].store_action = SG_STOREACTION_STORE;
    if (g_app->error_mode) {
      pass.colors[0].clear_value = {0.0f, 0.0f, 0.0f, 1.0f};
    } else {
      pass.colors[0].clear_value.r = g_app->renderer.clear_color[0];
      pass.colors[0].clear_value.g = g_app->renderer.clear_color[1];
      pass.colors[0].clear_value.b = g_app->renderer.clear_color[2];
      pass.colors[0].clear_value.a = g_app->renderer.clear_color[3];
    }
    sg_begin_default_pass(pass, sapp_width(), sapp_height());

    sgl_defaults();
    sgl_load_pipeline(g_app->pipeline);

    sgl_viewport(0, 0, sapp_width(), sapp_height(), true);
    sgl_ortho(0, sapp_widthf(), sapp_heightf(), 0, -1, 1);
  }

  if (g_app->error_mode) {
    if (g_app->default_font == nullptr) {
      g_app->default_font = (FontFamily *)mem_alloc(sizeof(FontFamily));
      font_load_default(g_app->default_font);
    }

    g_app->renderer.draw_colors[g_app->renderer.draw_colors_len - 1] = {
        255, 255, 255, 255};

    float x = 10;
    float y = 10;
    u64 font_size = 16;

    draw_font(&g_app->renderer, g_app->default_font, font_size, x, y,
              "oh no! there's an error! :(");
    y += font_size * 2;

    draw_font(&g_app->renderer, g_app->default_font, font_size, x, y,
              g_app->fatal_error);
    y += font_size * 2;

    if (g_app->traceback.data) {
      draw_font(&g_app->renderer, g_app->default_font, font_size, x, y,
                g_app->traceback);
    }
  } else {
    lua_State *L = g_app->L;
    lua_getglobal(L, "spry");

    lua_getfield(L, -1, "_timer_update");
    lua_pushnumber(L, g_app->time.delta);
    if (lua_pcall(L, 1, 0, 1) != LUA_OK) {
      lua_pop(L, 1);
    }

    {
      PROFILE_BLOCK("spry.frame");

      lua_getfield(L, -1, "frame");
      lua_pushnumber(L, g_app->time.delta);
      if (lua_pcall(L, 1, 0, 1) != LUA_OK) {
        lua_pop(L, 1);
      }
    }

    lua_pop(L, 1);
    assert(lua_gettop(L) == 1);
  }

  {
    PROFILE_BLOCK("end render pass");

    sgl_draw();

    sgl_error_t sgl_err = sgl_error();
    if (sgl_err != SGL_NO_ERROR) {
      panic("a draw error occurred: %d", sgl_err);
    }

    sg_end_pass();
    sg_commit();
  }

  memcpy(g_app->prev_key_state, g_app->key_state, sizeof(g_app->key_state));
  memcpy(g_app->prev_mouse_state, g_app->mouse_state,
         sizeof(g_app->mouse_state));
  g_app->prev_mouse_x = g_app->mouse_x;
  g_app->prev_mouse_y = g_app->mouse_y;
  g_app->scroll_x = 0;
  g_app->scroll_y = 0;

  Array<Sound *> &sounds = g_app->garbage_sounds;
  for (u64 i = 0; i < sounds.len;) {
    Sound *sound = sounds[i];

    if (sound->dead_end) {
      assert(sound->zombie);
      sound_trash(sound);
      mem_free(sound);

      sounds[i] = sounds[sounds.len - 1];
      sounds.len--;
    } else {
      i++;
    }
  }
}

static void actually_cleanup() {
  PROFILE_FUNC();

  lua_close(g_app->L);
  luaalloc_delete(g_app->LA);

  if (g_app->default_font != nullptr) {
    font_trash(g_app->default_font);
    mem_free(g_app->default_font);
  }

  for (Sound *sound : g_app->garbage_sounds) {
    sound_trash(sound);
    mem_free(sound);
  }
  array_trash(&g_app->garbage_sounds);
  ma_engine_uninit(&g_app->audio_engine);

  assets_shutdown();
  cute_mutex_destroy(&g_app->frame_mtx);

  sgl_destroy_pipeline(g_app->pipeline);
  sgl_shutdown();
  sg_shutdown();

  if (g_app->archive != nullptr) {
    g_app->archive->trash();
    mem_free(g_app->archive);
  }

  mem_free(g_app->fatal_error.data);
  mem_free(g_app->traceback.data);

  mem_free(g_app);
}

static void cleanup() {
  actually_cleanup();

#ifdef USE_PROFILER
  profile_shutdown();
#endif

#ifdef DEBUG
  DebugAllocator *allocator = dynamic_cast<DebugAllocator *>(g_allocator);
  if (allocator != nullptr) {
    i32 allocs = 0;
    for (DebugAllocInfo *info = allocator->head; info != nullptr;
         info = info->next) {
      printf("  %10llu bytes: %s:%d\n", (unsigned long long)info->size,
             info->file, info->line);
      allocs++;
    }
    printf("  --- %d allocation(s) ---\n", allocs);
  }
#endif

  delete g_allocator;

#ifdef DEBUG
  printf("bye\n");
#endif
}

static int spry_require_lua_script(lua_State *L) {
  PROFILE_FUNC();

  String path = luax_check_string(L, 1);

  Asset asset = {};
  bool ok = asset_load(AssetKind_LuaRef, g_app->archive, path, &asset);
  if (!ok) {
    return 0;
  }

  lua_rawgeti(L, LUA_REGISTRYINDEX, asset.lua_ref);
  return 1;
}

#ifdef __EMSCRIPTEN__
EM_JS(char *, web_mount_dir, (), { return stringToNewUTF8(spryMount); });

EM_ASYNC_JS(void, web_load_zip, (), {
  var dirs = spryMount.split('/');
  dirs.pop();

  var path = [];
  for (var dir of dirs) {
    path.push(dir);
    FS.mkdir(path.join('/'));
  }

  await fetch(spryMount).then(async function(res) {
    if (!res.ok) {
      throw new Error('failed to fetch ' + spryMount);
    }

    var data = await res.arrayBuffer();
    FS.writeFile(spryMount, new Uint8Array(data));
  });
});

EM_ASYNC_JS(void, web_load_files, (), {
  var jobs = [];

  function spryWalkFiles(files, leading) {
    var path = leading.join('/');
    if (path != '') {
      FS.mkdir(path);
    }

    for (var entry of Object.entries(files)) {
      var key = entry[0];
      var value = entry[1];
      var filepath = [... leading, key ];
      if (typeof value == 'object') {
        spryWalkFiles(value, filepath);
      } else if (value == 1) {
        var file = filepath.join('/');

        var job = fetch(file).then(async function(res) {
          if (!res.ok) {
            throw new Error('failed to fetch ' + file);
          }
          var data = await res.arrayBuffer();
          FS.writeFile(file, new Uint8Array(data));
        });

        jobs.push(job);
      }
    }
  }
  spryWalkFiles(spryFiles, []);

  await Promise.all(jobs);
});
#endif

static void setup_lua() {
  PROFILE_FUNC();

  LuaAlloc *LA = luaalloc_create(nullptr, nullptr);
  lua_State *L = lua_newstate(luaalloc, LA);

  g_app->LA = LA;
  g_app->L = L;

  luaL_openlibs(L);
  open_spry_api(L);

  // add error message handler. always at the bottom of stack.
  lua_pushcfunction(L, luax_msgh);

  lua_getglobal(L, "spry");
  lua_pushcfunction(L, spry_require_lua_script);
  lua_setfield(L, -2, "_require_lua_script");
  lua_pop(L, 1);

  const char *bootstrap =
#include "bootstrap.lua"
      ;

  if (luaL_loadbuffer(L, bootstrap, strlen(bootstrap), "bootstrap.lua") !=
      LUA_OK) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    panic("failed to load bootstrap");
  }

  if (lua_pcall(L, 0, 0, 1) != LUA_OK) {
    panic("failed to run bootstrap");
  }
}

static void mount_files(int argc, char **argv, bool *can_hot_reload) {
  PROFILE_FUNC();

#ifdef __EMSCRIPTEN__
  String mount_dir = web_mount_dir();
  defer(free(mount_dir.data));

  if (ends_with(mount_dir, ".zip")) {
    web_load_zip();
    g_app->archive = load_zip_archive(mount_dir);
  } else {
    web_load_files();
    g_app->archive = load_filesystem_archive(mount_dir);
  }
#else
  if (argc == 1) {
    String path = os_program_path();
#ifdef DEBUG
    printf("program path: %s\n", path.data);
#endif
    g_app->archive = load_zip_archive(path);
  } else if (argc == 2) {
    String mount_dir = argv[1];

    if (ends_with(mount_dir, ".zip")) {
      g_app->archive = load_zip_archive(mount_dir);
    } else {
      g_app->archive = load_filesystem_archive(mount_dir);
      *can_hot_reload = true;
    }
  }
#endif
}

static void load_all_lua_scripts(lua_State *L) {
  PROFILE_FUNC();

  Array<String> files = {};
  defer({
    for (String str : files) {
      mem_free(str.data);
    }
    array_trash(&files);
  });

  bool ok = g_app->archive->list_all_files(&files);
  if (!ok) {
    panic("failed to list all files");
  }
  qsort(files.data, files.len, sizeof(String),
        [](const void *a, const void *b) -> int {
          String *lhs = (String *)a;
          String *rhs = (String *)b;
          return strcmp(lhs->data, rhs->data);
        });

  for (String file : files) {
    if (file != "main.lua" && ends_with(file, ".lua")) {
      asset_load(AssetKind_LuaRef, g_app->archive, file, nullptr);
    }
  }
}

/* extern(app.h) */ App *g_app;
/* extern(prelude.h) */ Allocator *g_allocator;

sapp_desc sokol_main(int argc, char **argv) {
#ifdef DEBUG
  g_allocator = new DebugAllocator();
#else
  g_allocator = new HeapAllocator();
#endif

  profile_setup();
  PROFILE_FUNC();

  g_app = (App *)mem_alloc(sizeof(App));
  *g_app = {};

  setup_lua();
  lua_State *L = g_app->L;

  assets_setup();

  bool can_hot_reload = false;
  mount_files(argc, argv, &can_hot_reload);

  if (argc == 2 && g_app->archive == nullptr) {
    fatal_error(tmp_fmt("failed to load: %s", argv[1]));
  } else if (g_app->archive != nullptr) {
    asset_load(AssetKind_LuaRef, g_app->archive, "main.lua", nullptr);
  }

  lua_newtable(L);

  if (!g_app->error_mode) {
    lua_getglobal(L, "spry");
    lua_getfield(L, -1, "conf");
    lua_remove(L, -2);
    lua_pushvalue(L, -2); // same as lua_newtable above
    if (lua_pcall(L, 1, 0, 1) != LUA_OK) {
      lua_pop(L, 1);
    }
  }

#ifdef DEBUG
  bool win_console = luax_boolean_field(L, "win_console", true);
#else
  bool win_console = luax_boolean_field(L, "win_console", false);
#endif

  bool hot_reload = luax_boolean_field(L, "hot_reload", true);
  bool startup_load_scripts =
      luax_boolean_field(L, "startup_load_scripts", true);
  bool fullscreen = luax_boolean_field(L, "fullscreen", false);
  lua_Number reload_interval = luax_number_field(L, "reload_interval", 0.1);
  lua_Number swap_interval = luax_number_field(L, "swap_interval", 1);
  lua_Number target_fps = luax_number_field(L, "target_fps", 0);
  lua_Number width = luax_number_field(L, "window_width", 800);
  lua_Number height = luax_number_field(L, "window_height", 600);
  String title = luax_string_field(L, "window_title", "Spry");

  lua_pop(L, 1);

  if (startup_load_scripts && g_app->archive != nullptr) {
    load_all_lua_scripts(L);
  }

  cute_atomic_set(&g_app->hot_reload_enabled, can_hot_reload && hot_reload);
  cute_atomic_set(&g_app->reload_interval, (u32)(reload_interval * 1000));

  if (target_fps != 0) {
    g_app->time.target_ticks = 1000000000 / target_fps;
  }

#ifdef IS_WIN32
  if (!win_console) {
    FreeConsole();
  }
#endif

  sapp_desc sapp = {};
  sapp.init_cb = init;
  sapp.frame_cb = frame;
  sapp.cleanup_cb = cleanup;
  sapp.event_cb = event;
  sapp.width = (i32)width;
  sapp.height = (i32)height;
  sapp.window_title = title.data;
  sapp.logger.func = slog_func;
  sapp.swap_interval = (i32)swap_interval;
  sapp.fullscreen = fullscreen;
  sapp.allocator.alloc_fn = sokol_alloc;
  sapp.allocator.free_fn = sokol_free;

#ifdef __EMSCRIPTEN__
  sapp.gl_force_gles2 = true;
#endif

#ifdef DEBUG
  printf("debug build\n");
#endif
  return sapp;
}
