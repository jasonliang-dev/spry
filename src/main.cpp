#include "api.h"
#include "app.h"
#include "archive.h"
#include "atlas.h"
#include "audio.h"
#include "deps/lua/lauxlib.h"
#include "deps/lua/lua.h"
#include "deps/lua/lualib.h"
#include "deps/sokol_app.h"
#include "deps/sokol_audio.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "deps/sokol_glue.h"
#include "deps/sokol_log.h"
#include "deps/sokol_time.h"
#include "draw.h"
#include "font.h"
#include "hash_map.h"
#include "image.h"
#include "luax.h"
#include "prelude.h"
#include "scanner.h"
#include "sprite.h"
#include "strings.h"
#include "tilemap.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static lua_State *L = nullptr;
static sgl_pipeline g_pipeline;

static void fatal_error(String str) {
  g_app->fatal_error = to_cstr(str);
  fprintf(stderr, "%s\n", g_app->fatal_error.data);
  g_app->error_mode = true;
}

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
  sg_desc sg = {};
  sg.logger.func = slog_func;
  sg.context = sapp_sgcontext();
  sg.allocator.alloc = sokol_alloc;
  sg.allocator.free = sokol_free;
  sg_setup(sg);

  sgl_desc_t sgl = {};
  sgl.logger.func = slog_func;
  sgl.allocator.alloc = sokol_alloc;
  sgl.allocator.free = sokol_free;
  sgl_setup(sgl);

  saudio_desc saudio = {};
  saudio.logger.func = slog_func;
  saudio.num_channels = 2;
  saudio.allocator.alloc = sokol_alloc;
  saudio.allocator.free = sokol_free;
  saudio_setup(saudio);

  sg_pipeline_desc sg_pipline = {};
  sg_pipline.depth.write_enabled = true;
  sg_pipline.colors[0].blend.enabled = true;
  sg_pipline.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
  sg_pipline.colors[0].blend.dst_factor_rgb =
      SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
  g_pipeline = sgl_make_pipeline(sg_pipline);

  stm_setup();
  g_app->time_begin = stm_now();

  g_app->clear_color[0] = 0.0f;
  g_app->clear_color[1] = 0.0f;
  g_app->clear_color[2] = 0.0f;
  g_app->clear_color[3] = 1.0f;

  g_app->draw_colors[0].r = 255;
  g_app->draw_colors[0].g = 255;
  g_app->draw_colors[0].b = 255;
  g_app->draw_colors[0].a = 255;
  g_app->draw_colors_len = 1;

  if (!g_app->error_mode) {
    lua_getglobal(L, "spry");
    lua_getfield(L, -1, "start");
    lua_remove(L, -2);
    if (lua_pcall(L, 0, 0, 1) != LUA_OK) {
      lua_pop(L, 1);
    }
  }

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
    g_app->mouse_dx = e->mouse_dx;
    g_app->mouse_dy = e->mouse_dy;
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
  u64 time_now = stm_now();
  g_app->delta_time = stm_sec(time_now - g_app->time_begin);
  g_app->time_begin = time_now;

  i32 frames = saudio_expect();
  i32 channels = saudio_channels();
  i32 samples = frames * channels;
  if (g_app->audio_buffer.capacity < samples) {
    reserve(&g_app->audio_buffer, samples);
  }

  if (samples > 0) {
    audio_playback(&g_app->audio_sources, g_app->audio_buffer.data, frames,
                   channels, g_app->master_volume);
    saudio_push(g_app->audio_buffer.data, samples);
  }

  sg_pass_action pass = {};
  pass.colors[0].action = SG_ACTION_CLEAR;
  if (g_app->error_mode) {
    pass.colors[0].value = {0.0f, 0.0f, 0.0f, 1.0f};
  } else {
    pass.colors[0].value.r = g_app->clear_color[0];
    pass.colors[0].value.g = g_app->clear_color[1];
    pass.colors[0].value.b = g_app->clear_color[2];
    pass.colors[0].value.a = g_app->clear_color[3];
  }
  sg_begin_default_pass(pass, sapp_width(), sapp_height());

  sgl_defaults();
  sgl_load_pipeline(g_pipeline);

  sgl_viewport(0, 0, sapp_width(), sapp_height(), true);
  sgl_ortho(0, sapp_widthf(), sapp_heightf(), 0, -1, 1);

  if (g_app->error_mode) {
    if (!g_app->default_font_loaded) {
      g_app->default_font = (FontFamily *)mem_alloc(sizeof(FontFamily));
      font_load_default(g_app->default_font);
      g_app->default_font_loaded = true;
    }

    float x = 10;
    float y = 10;
    u64 font_size = 16;

    Color color = {255, 255, 255, 255};
    draw(g_app->default_font, font_size, x, y, "oh no! there's an error! :(",
         color);
    y += font_size * 2;

    draw(g_app->default_font, font_size, x, y, g_app->fatal_error, color);
    y += font_size * 2;

    if (g_app->traceback.data) {
      draw(g_app->default_font, font_size, x, y, g_app->traceback, color);
    }
  }

  lua_getglobal(L, "spry");

  if (!g_app->error_mode) {
    lua_getfield(L, -1, "timer_update");
    lua_pushnumber(L, g_app->delta_time);
    if (lua_pcall(L, 1, 0, 1) != LUA_OK) {
      lua_pop(L, 1);
    }
  }

  if (!g_app->error_mode) {
    lua_getfield(L, -1, "frame");
    lua_pushnumber(L, g_app->delta_time);
    if (lua_pcall(L, 1, 0, 1) != LUA_OK) {
      lua_pop(L, 1);
    }
  }

  lua_pop(L, 1);

  if (!g_app->error_mode) {
    assert(lua_gettop(L) == 1);
  }

  sgl_draw();

  sgl_error_t sgl_err = sgl_error();
  if (sgl_err != SGL_NO_ERROR) {
    panic("a draw error occurred: %d", sgl_err);
  }

  sg_end_pass();
  sg_commit();

  memcpy(g_app->prev_key_state, g_app->key_state, sizeof(g_app->key_state));
  memcpy(g_app->prev_mouse_state, g_app->mouse_state,
         sizeof(g_app->mouse_state));
  g_app->mouse_dx = 0;
  g_app->mouse_dy = 0;
  g_app->scroll_x = 0;
  g_app->scroll_y = 0;
}

static void dump_allocs(Allocator *a) {
  i32 allocs = 0;
  for (DebugAllocInfo *info = a->head; info != nullptr; info = info->next) {
    allocs++;
  }

  printf("  --- allocations (%d) ---\n", allocs);
  for (DebugAllocInfo *info = a->head; info != nullptr; info = info->next) {
    printf("  %10llu bytes: %s:%d\n", info->size, info->file, info->line);
  }
}

static void cleanup() {
  for (auto [k, v] : g_app->modules) {
    mem_free(v->name.data);
  }
  drop(&g_app->modules);

  lua_close(L);

  saudio_shutdown();

  drop(&g_app->audio_buffer);
  drop(&g_app->audio_sources);

  if (g_app->default_font_loaded) {
    drop(g_app->default_font);
    mem_free(g_app->default_font);
  }

  for (auto [k, v] : g_app->sprites) {
    drop(v);
  }
  drop(&g_app->sprites);

  sgl_destroy_pipeline(g_pipeline);
  sgl_shutdown();
  sg_shutdown();

  drop(&g_app->archive);

  if (g_app->fatal_error.data) {
    mem_free(g_app->fatal_error.data);
  }

  if (g_app->traceback.data) {
    mem_free(g_app->traceback.data);
  }

  mem_free(g_app);

#ifdef DEBUG
  dump_allocs(&g_allocator);
#endif
}

static i32 require_lua_script(Archive *ar, String filepath) {
  if (g_app->error_mode) {
    return LUA_REFNIL;
  }

  String path = to_cstr(filepath);
  defer(mem_free(path.data));

  Module *module = get(&g_app->modules, fnv1a(path));
  if (module != nullptr) {
    return module->ref;
  }

  String contents;
  bool ok = ar->read_entire_file(ar, &contents, filepath);
  if (!ok) {
    StringBuilder sb = format("failed to read file: %s", path.data);
    defer(drop(&sb));
    fatal_error(as_string(&sb));
    return LUA_REFNIL;
  }
  defer(mem_free(contents.data));

  // [1] {}
  lua_newtable(L);
  i32 table_index = lua_gettop(L);

  if (luaL_loadbuffer(L, contents.data, contents.len, path.data) != LUA_OK) {
    fatal_error(luax_check_string(L, -1));
    return LUA_REFNIL;
  }

  // [1] {}
  // ...
  // [n] any
  if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {
    lua_pop(L, 2);
    return LUA_REFNIL;
  }

  // [1] {...}
  i32 top = lua_gettop(L);
  for (i32 i = 1; i <= top - table_index; i++) {
    lua_seti(L, table_index, i);
  }

  Module m = {};
  m.name = to_cstr(filepath);
  m.ref = luaL_ref(L, LUA_REGISTRYINDEX);
  g_app->modules[fnv1a(path)] = m;

  return m.ref;
}

static int require_lua_script(lua_State *L) {
  String path = luax_check_string(L, 1);
  i32 ref = require_lua_script(&g_app->archive, path);

  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
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

static int string_cmp(const void *a, const void *b) {
  String *lhs = (String *)a;
  String *rhs = (String *)b;
  return strcmp(lhs->data, rhs->data);
}

sapp_desc sokol_main(int argc, char **argv) {
  g_app = (App *)mem_alloc(sizeof(App));
  *g_app = {};

  L = luaL_newstate();

  luaL_openlibs(L);
  open_spry_api(L);

  // add error message handler. always at the bottom of stack.
  lua_pushcfunction(L, luax_msgh);

  lua_getglobal(L, "spry");
  lua_pushcfunction(L, require_lua_script);
  lua_setfield(L, -2, "require_lua_script");
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

  bool ok = false;

#ifdef __EMSCRIPTEN__
  String mount_dir = web_mount_dir();
  defer(free(mount_dir.data));

  if (ends_with(mount_dir, ".zip")) {
    web_load_zip();
    ok = load_zip_archive(&g_app->archive, mount_dir);
  } else {
    web_load_files();
    ok = load_filesystem_archive(&g_app->archive, mount_dir);
  }
#else
  if (argc == 1) {
    String path = program_path();
#ifdef DEBUG
    printf("program path: %s\n", path.data);
#endif
    ok = load_zip_archive(&g_app->archive, path);
    g_app->mounted = ok;
  } else if (argc == 2) {
    String mount_dir = argv[1];

    if (ends_with(mount_dir, ".zip")) {
      ok = load_zip_archive(&g_app->archive, mount_dir);
    } else {
      ok = load_filesystem_archive(&g_app->archive, mount_dir);
    }

    g_app->mounted = true;
  }
#endif

  if (g_app->mounted) {
    if (!ok) {
      StringBuilder sb = format("failed to mount: %s", argv[1]);
      defer(drop(&sb));

      fatal_error(as_string(&sb));
    } else {
      Array<String> files = {};
      defer({
        for (String str : files) {
          mem_free(str.data);
        }
        drop(&files);
      });

      ok = g_app->archive.list_all_files(&g_app->archive, &files);
      if (!ok) {
        panic("failed to list all files");
      }
      qsort(files.data, files.len, sizeof(String), string_cmp);

      for (String file : files) {
        if (file != "main.lua" && ends_with(file, ".lua")) {
          require_lua_script(&g_app->archive, file);
        }
      }
      require_lua_script(&g_app->archive, "main.lua");
    }
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

  bool console_attach = luax_boolean_field(L, "console_attach", false);
  lua_Number swap_interval = luax_number_field(L, "swap_interval", 1);
  lua_Number width = luax_number_field(L, "window_width", 800);
  lua_Number height = luax_number_field(L, "window_height", 600);
  String title = luax_string_field(L, "window_title", "Spry");

  lua_pop(L, 1);

  sapp_desc sapp = {};
  sapp.init_cb = init;
  sapp.frame_cb = frame;
  sapp.cleanup_cb = cleanup;
  sapp.event_cb = event;
  sapp.width = (i32)width;
  sapp.height = (i32)height;
  sapp.window_title = title.data;
  sapp.logger.func = slog_func;
  sapp.win32_console_attach = console_attach;
  sapp.swap_interval = (i32)swap_interval;
  sapp.allocator.alloc = sokol_alloc;
  sapp.allocator.free = sokol_free;

#ifdef __EMSCRIPTEN__
  sapp.gl_force_gles2 = true;
#endif

#ifdef DEBUG
  printf("debug build\n");
#endif
  return sapp;
}
