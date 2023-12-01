#include "api.h"
#include "app.h"
#include "array.h"
#include "assets.h"
#include "atlas.h"
#include "deps/microui.h"
#include "deps/sokol_app.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "deps/sokol_time.h"
#include "draw.h"
#include "font.h"
#include "image.h"
#include "json.h"
#include "luax.h"
#include "microui.h"
#include "os.h"
#include "physics.h"
#include "prelude.h"
#include "profile.h"
#include "sound.h"
#include "sprite.h"
#include "stb_decompress.h"
#include "tilemap.h"
#include <box2d/box2d.h>

#ifndef IS_HTML5
#include "embed/ltn12_compressed.h"
#include "embed/mbox_compressed.h"
#include "embed/mime_compressed.h"
#include "embed/socket_compressed.h"
#include "embed/socket_ftp_compressed.h"
#include "embed/socket_headers_compressed.h"
#include "embed/socket_http_compressed.h"
#include "embed/socket_smtp_compressed.h"
#include "embed/socket_tp_compressed.h"
#include "embed/socket_url_compressed.h"
#endif

extern "C" {
#include "deps/luasocket/luasocket.h"
#include "deps/luasocket/mime.h"
#include <lauxlib.h>
#include <lua.h>
}

// mt_sampler

static int mt_sampler_gc(lua_State *L) {
  u32 *udata = (u32 *)luaL_checkudata(L, 1, "mt_sampler");
  u32 id = *udata;

  if (id != SG_INVALID_ID) {
    sg_destroy_sampler({id});
  }

  return 0;
}

static int mt_sampler_use(lua_State *L) {
  u32 *id = (u32 *)luaL_checkudata(L, 1, "mt_sampler");
  renderer_use_sampler(*id);
  return 0;
}

static int open_mt_sampler(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_sampler_gc},
      {"use", mt_sampler_use},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_sampler", reg);
  return 0;
}

// mt_image

static int mt_image_draw(lua_State *L) {
  Image img = check_asset_mt(L, 1, "mt_image").image;

  DrawDescription dd = draw_description_args(L, 2);
  draw_image(&img, &dd);
  return 0;
}

static int mt_image_width(lua_State *L) {
  Image img = check_asset_mt(L, 1, "mt_image").image;
  lua_pushnumber(L, img.width);
  return 1;
}

static int mt_image_height(lua_State *L) {
  Image img = check_asset_mt(L, 1, "mt_image").image;
  lua_pushnumber(L, img.height);
  return 1;
}

static int open_mt_image(lua_State *L) {
  luaL_Reg reg[] = {
      {"draw", mt_image_draw},
      {"width", mt_image_width},
      {"height", mt_image_height},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_image", reg);
  return 0;
}

// mt_font

static int mt_font_gc(lua_State *L) {
  FontFamily **udata = (FontFamily **)luaL_checkudata(L, 1, "mt_font");
  FontFamily *font = *udata;

  if (font != g_app->default_font) {
    font_trash(font);
    mem_free(font);
  }
  return 0;
}

static int mt_font_width(lua_State *L) {
  FontFamily **udata = (FontFamily **)luaL_checkudata(L, 1, "mt_font");
  FontFamily *font = *udata;

  String text = luax_check_string(L, 2);
  lua_Number size = luaL_checknumber(L, 3);

  float w = font_width(font, size, text);

  lua_pushnumber(L, w);
  return 1;
}

static int mt_font_draw(lua_State *L) {
  FontFamily **udata = (FontFamily **)luaL_checkudata(L, 1, "mt_font");
  FontFamily *font = *udata;

  String text = luax_check_string(L, 2);
  lua_Number x = luaL_optnumber(L, 3, 0);
  lua_Number y = luaL_optnumber(L, 4, 0);
  lua_Number size = luaL_optnumber(L, 5, 12);
  lua_Number wrap = luaL_optnumber(L, 6, -1);

  float bottom = 0;
  if (wrap < 0) {
    bottom = draw_font(font, (u64)size, (float)x, (float)y, text);
  } else {
    bottom = draw_font_wrapped(font, (u64)size, (float)x, (float)y, text,
                               (float)wrap);
  }

  lua_pushnumber(L, bottom);
  return 1;
}

static int open_mt_font(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_font_gc},
      {"width", mt_font_width},
      {"draw", mt_font_draw},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_font", reg);
  return 0;
}

// mt_sound

static ma_sound *sound_ma(lua_State *L) {
  Sound **udata = (Sound **)luaL_checkudata(L, 1, "mt_sound");
  Sound *sound = *udata;
  return &sound->ma;
}

static int mt_sound_gc(lua_State *L) {
  Sound **udata = (Sound **)luaL_checkudata(L, 1, "mt_sound");
  Sound *sound = *udata;

  if (ma_sound_at_end(&sound->ma)) {
    sound_trash(sound);
  } else {
    sound->zombie = true;
    array_push(&g_app->garbage_sounds, sound);
  }

  return 0;
}

static int mt_sound_frames(lua_State *L) {
  unsigned long long frames = 0;
  ma_result res = ma_sound_get_length_in_pcm_frames(sound_ma(L), &frames);
  if (res != MA_SUCCESS) {
    return 0;
  }

  lua_pushinteger(L, (lua_Integer)frames);
  return 1;
}

static int mt_sound_start(lua_State *L) {
  ma_result res = ma_sound_start(sound_ma(L));
  if (res != MA_SUCCESS) {
    luaL_error(L, "failed to start sound");
  }

  return 0;
}

static int mt_sound_stop(lua_State *L) {
  ma_result res = ma_sound_stop(sound_ma(L));
  if (res != MA_SUCCESS) {
    luaL_error(L, "failed to stop sound");
  }

  return 0;
}

static int mt_sound_seek(lua_State *L) {
  lua_Number f = luaL_optnumber(L, 2, 0);

  ma_result res = ma_sound_seek_to_pcm_frame(sound_ma(L), f);
  if (res != MA_SUCCESS) {
    luaL_error(L, "failed to seek to frame");
  }

  return 0;
}

static int mt_sound_secs(lua_State *L) {
  float len = 0;
  ma_result res = ma_sound_get_length_in_seconds(sound_ma(L), &len);
  if (res != MA_SUCCESS) {
    return 0;
  }

  lua_pushnumber(L, len);
  return 1;
}

static int mt_sound_vol(lua_State *L) {
  lua_pushnumber(L, ma_sound_get_volume(sound_ma(L)));
  return 1;
}

static int mt_sound_set_vol(lua_State *L) {
  ma_sound_set_volume(sound_ma(L), (float)luaL_optnumber(L, 2, 0));
  return 0;
}

static int mt_sound_pan(lua_State *L) {
  lua_pushnumber(L, ma_sound_get_pan(sound_ma(L)));
  return 1;
}

static int mt_sound_set_pan(lua_State *L) {
  ma_sound_set_pan(sound_ma(L), (float)luaL_optnumber(L, 2, 0));
  return 0;
}

static int mt_sound_pitch(lua_State *L) {
  lua_pushnumber(L, ma_sound_get_pitch(sound_ma(L)));
  return 1;
}

static int mt_sound_set_pitch(lua_State *L) {
  ma_sound_set_pitch(sound_ma(L), (float)luaL_optnumber(L, 2, 0));
  return 0;
}

static int mt_sound_loop(lua_State *L) {
  lua_pushboolean(L, ma_sound_is_looping(sound_ma(L)));
  return 1;
}

static int mt_sound_set_loop(lua_State *L) {
  ma_sound_set_looping(sound_ma(L), lua_toboolean(L, 2));
  return 0;
}

static int mt_sound_pos(lua_State *L) {
  ma_vec3f pos = ma_sound_get_position(sound_ma(L));
  lua_pushnumber(L, pos.x);
  lua_pushnumber(L, pos.y);
  return 2;
}

static int mt_sound_set_pos(lua_State *L) {
  lua_Number x = luaL_optnumber(L, 2, 0);
  lua_Number y = luaL_optnumber(L, 3, 0);
  ma_sound_set_position(sound_ma(L), (float)x, (float)y, 0.0f);
  return 0;
}

static int mt_sound_dir(lua_State *L) {
  ma_vec3f dir = ma_sound_get_direction(sound_ma(L));
  lua_pushnumber(L, dir.x);
  lua_pushnumber(L, dir.y);
  return 2;
}

static int mt_sound_set_dir(lua_State *L) {
  lua_Number x = luaL_optnumber(L, 2, 0);
  lua_Number y = luaL_optnumber(L, 3, 0);
  ma_sound_set_direction(sound_ma(L), (float)x, (float)y, 0.0f);
  return 0;
}

static int mt_sound_vel(lua_State *L) {
  ma_vec3f vel = ma_sound_get_velocity(sound_ma(L));
  lua_pushnumber(L, vel.x);
  lua_pushnumber(L, vel.y);
  return 2;
}

static int mt_sound_set_vel(lua_State *L) {
  lua_Number x = luaL_optnumber(L, 2, 0);
  lua_Number y = luaL_optnumber(L, 3, 0);
  ma_sound_set_velocity(sound_ma(L), (float)x, (float)y, 0.0f);
  return 0;
}

static int mt_sound_set_fade(lua_State *L) {
  lua_Number from = luaL_optnumber(L, 2, 0);
  lua_Number to = luaL_optnumber(L, 3, 0);
  lua_Number ms = luaL_optnumber(L, 4, 0);
  ma_sound_set_fade_in_milliseconds(sound_ma(L), (float)from, (float)to,
                                    (u64)ms);
  return 0;
}

static int open_mt_sound(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_sound_gc},           {"frames", mt_sound_frames},
      {"secs", mt_sound_secs},         {"start", mt_sound_start},
      {"stop", mt_sound_stop},         {"seek", mt_sound_seek},
      {"vol", mt_sound_vol},           {"set_vol", mt_sound_set_vol},
      {"pan", mt_sound_pan},           {"set_pan", mt_sound_set_pan},
      {"pitch", mt_sound_pitch},       {"set_pitch", mt_sound_set_pitch},
      {"loop", mt_sound_loop},         {"set_loop", mt_sound_set_loop},
      {"pos", mt_sound_pos},           {"set_pos", mt_sound_set_pos},
      {"dir", mt_sound_dir},           {"set_dir", mt_sound_set_dir},
      {"vel", mt_sound_vel},           {"set_vel", mt_sound_set_vel},
      {"set_fade", mt_sound_set_fade}, {nullptr, nullptr},
  };

  luax_new_class(L, "mt_sound", reg);
  return 0;
}

// mt_sprite

static int mt_sprite_play(lua_State *L) {
  Sprite *spr = (Sprite *)luaL_checkudata(L, 1, "mt_sprite");
  String tag = luax_check_string(L, 2);
  bool restart = lua_toboolean(L, 3);

  bool same = sprite_play(spr, tag);
  if (!same || restart) {
    spr->current_frame = 0;
    spr->elapsed = 0;
  }
  return 0;
}

static int mt_sprite_update(lua_State *L) {
  Sprite *spr = (Sprite *)luaL_checkudata(L, 1, "mt_sprite");
  lua_Number dt = luaL_checknumber(L, 2);

  sprite_update(spr, (float)dt);
  return 0;
}

static int mt_sprite_draw(lua_State *L) {
  Sprite *spr = (Sprite *)luaL_checkudata(L, 1, "mt_sprite");
  DrawDescription dd = draw_description_args(L, 2);

  draw_sprite(spr, &dd);
  return 0;
}

static int mt_sprite_width(lua_State *L) {
  Sprite *spr = (Sprite *)luaL_checkudata(L, 1, "mt_sprite");
  SpriteData data = check_asset(L, spr->sprite).sprite;

  lua_pushnumber(L, (lua_Number)data.width);
  return 1;
}

static int mt_sprite_height(lua_State *L) {
  Sprite *spr = (Sprite *)luaL_checkudata(L, 1, "mt_sprite");
  SpriteData data = check_asset(L, spr->sprite).sprite;

  lua_pushnumber(L, (lua_Number)data.height);
  return 1;
}

static int mt_sprite_set_frame(lua_State *L) {
  Sprite *spr = (Sprite *)luaL_checkudata(L, 1, "mt_sprite");
  lua_Integer frame = luaL_checknumber(L, 2);

  sprite_set_frame(spr, (i32)frame);
  return 0;
}

static int mt_sprite_total_frames(lua_State *L) {
  Sprite *spr = (Sprite *)luaL_checkudata(L, 1, "mt_sprite");
  SpriteData data = check_asset(L, spr->sprite).sprite;

  lua_pushinteger(L, data.frames.len);
  return 1;
}

static int open_mt_sprite(lua_State *L) {
  luaL_Reg reg[] = {
      {"play", mt_sprite_play},
      {"update", mt_sprite_update},
      {"draw", mt_sprite_draw},
      {"width", mt_sprite_width},
      {"height", mt_sprite_height},
      {"set_frame", mt_sprite_set_frame},
      {"total_frames", mt_sprite_total_frames},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_sprite", reg);
  return 0;
}

// mt_atlas_image

static int mt_atlas_image_draw(lua_State *L) {
  AtlasImage *atlas_img = (AtlasImage *)luaL_checkudata(L, 1, "mt_atlas_image");
  DrawDescription dd = draw_description_args(L, 2);

  dd.u0 = atlas_img->u0;
  dd.v0 = atlas_img->v0;
  dd.u1 = atlas_img->u1;
  dd.v1 = atlas_img->v1;

  draw_image(&atlas_img->img, &dd);
  return 0;
}

static int mt_atlas_image_width(lua_State *L) {
  AtlasImage *atlas_img = (AtlasImage *)luaL_checkudata(L, 1, "mt_atlas_image");
  lua_pushnumber(L, atlas_img->width);
  return 1;
}

static int mt_atlas_image_height(lua_State *L) {
  AtlasImage *atlas_img = (AtlasImage *)luaL_checkudata(L, 1, "mt_atlas_image");
  lua_pushnumber(L, atlas_img->height);
  return 1;
}

static int open_mt_atlas_image(lua_State *L) {
  luaL_Reg reg[] = {
      {"draw", mt_atlas_image_draw},
      {"width", mt_atlas_image_width},
      {"height", mt_atlas_image_height},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_atlas_image", reg);
  return 0;
}

// mt_atlas

static int mt_atlas_gc(lua_State *L) {
  Atlas *atlas = (Atlas *)luaL_checkudata(L, 1, "mt_atlas");
  atlas_trash(atlas);
  return 0;
}

static int mt_atlas_get_image(lua_State *L) {
  Atlas *atlas = (Atlas *)luaL_checkudata(L, 1, "mt_atlas");
  String name = luax_check_string(L, 2);

  AtlasImage *atlas_img = atlas_get(atlas, name);
  if (atlas_img == nullptr) {
    return 0;
  }

  luax_new_userdata(L, *atlas_img, "mt_atlas_image");
  return 1;
}

static int open_mt_atlas(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_atlas_gc},
      {"get_image", mt_atlas_get_image},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_atlas", reg);
  return 0;
}

// mt_tilemap

static int mt_tilemap_draw(lua_State *L) {
  Tilemap tm = check_asset_mt(L, 1, "mt_tilemap").tilemap;
  draw_tilemap(&tm);
  return 0;
}

static int mt_tilemap_entities(lua_State *L) {
  Tilemap tm = check_asset_mt(L, 1, "mt_tilemap").tilemap;

  u64 entities = 0;
  for (TilemapLevel &level : tm.levels) {
    for (TilemapLayer &layer : level.layers) {
      entities += layer.entities.len;
    }
  }

  lua_createtable(L, (i32)entities, 0);

  i32 i = 1;
  for (TilemapLevel &level : tm.levels) {
    for (TilemapLayer &layer : level.layers) {
      for (TilemapEntity &entity : layer.entities) {
        lua_createtable(L, 0, 3);

        luax_set_string_field(L, "id", entity.identifier.data);
        luax_set_number_field(L, "x", entity.x + level.world_x);
        luax_set_number_field(L, "y", entity.y + level.world_y);

        lua_rawseti(L, -2, i);
        i++;
      }
    }
  }

  return 1;
}

static int mt_tilemap_make_collision(lua_State *L) {
  Asset asset = check_asset_mt(L, 1, "mt_tilemap");
  defer(asset_write(asset));

  Physics *physics = (Physics *)luaL_checkudata(L, 2, "mt_b2_world");
  String name = luax_check_string(L, 3);

  Array<TilemapInt> walls = {};
  defer(array_trash(&walls));

  array_reserve(&walls, luax_len(L, 4));
  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    lua_Number tile = luaL_checknumber(L, -1);
    array_push(&walls, (TilemapInt)tile);
  }

  tilemap_make_collision(&asset.tilemap, physics->world, physics->meter, name,
                         walls);
  return 0;
}

static int mt_tilemap_draw_fixtures(lua_State *L) {
  Tilemap tm = check_asset_mt(L, 1, "mt_tilemap").tilemap;
  Physics *physics = (Physics *)luaL_checkudata(L, 2, "mt_b2_world");
  String name = luax_check_string(L, 3);

  b2Body **body = hashmap_get(&tm.bodies, fnv1a(name));
  if (body != nullptr) {
    draw_fixtures_for_body(*body, physics->meter);
  }

  return 0;
}

static int mt_tilemap_make_graph(lua_State *L) {
  Asset asset = check_asset_mt(L, 1, "mt_tilemap");
  defer(asset_write(asset));

  String name = luax_check_string(L, 2);
  i32 bloom = (i32)luaL_optnumber(L, 4, 1);

  Array<TileCost> costs = {};
  defer(array_trash(&costs));

  lua_pushvalue(L, 3);

  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    lua_Number value = luaL_checknumber(L, -1);
    lua_Number key = luaL_checknumber(L, -2);

    TileCost cost = {};
    cost.cell = (TilemapInt)key;
    cost.value = (float)value;

    array_push(&costs, cost);
  }
  lua_pop(L, 1);

  tilemap_make_graph(&asset.tilemap, bloom, name, costs);
  return 0;
}

static int mt_tilemap_astar(lua_State *L) {
  PROFILE_FUNC();

  Asset asset = check_asset_mt(L, 1, "mt_tilemap");
  defer(asset_write(asset));

  lua_Number sx = luaL_checknumber(L, 2);
  lua_Number sy = luaL_checknumber(L, 3);
  lua_Number ex = luaL_checknumber(L, 4);
  lua_Number ey = luaL_checknumber(L, 5);

  TilePoint start = {};
  start.x = (i32)sx;
  start.y = (i32)sy;

  TilePoint goal = {};
  goal.x = (i32)ex;
  goal.y = (i32)ey;

  TileNode *end = tilemap_astar(&asset.tilemap, goal, start);

  {
    PROFILE_BLOCK("construct path");

    lua_newtable(L);

    i32 i = 1;
    for (TileNode *n = end; n != nullptr; n = n->prev) {
      lua_createtable(L, 0, 2);

      luax_set_number_field(L, "x", n->x * asset.tilemap.graph_grid_size);
      luax_set_number_field(L, "y", n->y * asset.tilemap.graph_grid_size);

      lua_rawseti(L, -2, i);
      i++;
    }
  }

  return 1;
}

static int open_mt_tilemap(lua_State *L) {
  luaL_Reg reg[] = {
      {"draw", mt_tilemap_draw},
      {"entities", mt_tilemap_entities},
      {"make_collision", mt_tilemap_make_collision},
      {"draw_fixtures", mt_tilemap_draw_fixtures},
      {"make_graph", mt_tilemap_make_graph},
      {"astar", mt_tilemap_astar},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_tilemap", reg);
  return 0;
}

// box2d fixture

static int mt_b2_fixture_friction(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
  b2Fixture *fixture = physics->fixture;

  float friction = fixture->GetFriction();
  lua_pushnumber(L, friction);
  return 1;
}

static int mt_b2_fixture_restitution(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
  b2Fixture *fixture = physics->fixture;

  float restitution = fixture->GetRestitution();
  lua_pushnumber(L, restitution);
  return 1;
}

static int mt_b2_fixture_is_sensor(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
  b2Fixture *fixture = physics->fixture;

  float sensor = fixture->IsSensor();
  lua_pushnumber(L, sensor);
  return 1;
}

static int mt_b2_fixture_set_friction(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
  b2Fixture *fixture = physics->fixture;

  float friction = luaL_checknumber(L, 2);
  fixture->SetFriction(friction);
  return 0;
}

static int mt_b2_fixture_set_restitution(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
  b2Fixture *fixture = physics->fixture;

  float restitution = luaL_checknumber(L, 2);
  fixture->SetRestitution(restitution);
  return 0;
}

static int mt_b2_fixture_set_sensor(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
  b2Fixture *fixture = physics->fixture;

  bool sensor = lua_toboolean(L, 2);
  fixture->SetSensor(sensor);
  return 0;
}

static int mt_b2_fixture_body(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
  b2Fixture *fixture = physics->fixture;

  b2Body *body = fixture->GetBody();

  PhysicsUserData *pud = (PhysicsUserData *)body->GetUserData().pointer;
  assert(pud != nullptr);
  pud->ref_count++;

  Physics p = physics_weak_copy(physics);
  p.body = body;

  luax_new_userdata(L, p, "mt_b2_body");
  return 1;
}

static int mt_b2_fixture_udata(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_fixture");
  b2Fixture *fixture = physics->fixture;

  physics_push_userdata(L, fixture->GetUserData().pointer);
  return 1;
}

static int open_mt_b2_fixture(lua_State *L) {
  luaL_Reg reg[] = {
      {"friction", mt_b2_fixture_friction},
      {"restitution", mt_b2_fixture_restitution},
      {"is_sensor", mt_b2_fixture_is_sensor},
      {"set_friction", mt_b2_fixture_set_friction},
      {"set_restitution", mt_b2_fixture_set_restitution},
      {"set_sensor", mt_b2_fixture_set_sensor},
      {"body", mt_b2_fixture_body},
      {"udata", mt_b2_fixture_udata},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_b2_fixture", reg);
  return 0;
}

// box2d body

static int b2_body_unref(lua_State *L, bool destroy) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  if (physics->body != nullptr) {
    PhysicsUserData *pud =
        (PhysicsUserData *)physics->body->GetUserData().pointer;
    assert(pud != nullptr);
    pud->ref_count--;

    if (pud->ref_count == 0 || destroy) {
      physics_destroy_body(L, physics);
    }
  }

  return 0;
}

static int mt_b2_body_gc(lua_State *L) { return b2_body_unref(L, false); }

static int mt_b2_body_destroy(lua_State *L) { return b2_body_unref(L, true); }

static b2FixtureDef b2_fixture_def(lua_State *L, i32 arg) {
  bool sensor = luax_boolean_field(L, arg, "sensor");
  lua_Number density = luax_opt_number_field(L, arg, "density", 1);
  lua_Number friction = luax_opt_number_field(L, arg, "friction", 0.2);
  lua_Number restitution = luax_opt_number_field(L, arg, "restitution", 0);
  PhysicsUserData *pud = physics_userdata(L);

  b2FixtureDef def = {};
  def.isSensor = sensor;
  def.density = (float)density;
  def.friction = (float)friction;
  def.restitution = (float)restitution;
  def.userData.pointer = (u64)pud;
  return def;
}

static int mt_b2_body_make_box_fixture(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;
  b2FixtureDef fixture_def = b2_fixture_def(L, 2);

  lua_Number x = luax_opt_number_field(L, 2, "x", 0);
  lua_Number y = luax_opt_number_field(L, 2, "y", 0);
  lua_Number w = luax_number_field(L, 2, "w");
  lua_Number h = luax_number_field(L, 2, "h");
  lua_Number angle = luax_opt_number_field(L, 2, "angle", 0);

  b2Vec2 pos = {(float)x / physics->meter, (float)y / physics->meter};

  b2PolygonShape box = {};
  box.SetAsBox((float)w / physics->meter, (float)h / physics->meter, pos,
               angle);
  fixture_def.shape = &box;

  Physics p = physics_weak_copy(physics);
  p.fixture = body->CreateFixture(&fixture_def);

  luax_new_userdata(L, p, "mt_b2_fixture");
  return 0;
}

static int mt_b2_body_make_circle_fixture(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;
  b2FixtureDef fixture_def = b2_fixture_def(L, 2);

  lua_Number x = luax_opt_number_field(L, 2, "x", 0);
  lua_Number y = luax_opt_number_field(L, 2, "y", 0);
  lua_Number radius = luax_number_field(L, 2, "radius");

  b2CircleShape circle = {};
  circle.m_radius = radius / physics->meter;
  circle.m_p = {(float)x / physics->meter, (float)y / physics->meter};
  fixture_def.shape = &circle;

  Physics p = physics_weak_copy(physics);
  p.fixture = body->CreateFixture(&fixture_def);

  luax_new_userdata(L, p, "mt_b2_fixture");
  return 0;
}

static int mt_b2_body_position(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  b2Vec2 pos = body->GetPosition();

  lua_pushnumber(L, pos.x * physics->meter);
  lua_pushnumber(L, pos.y * physics->meter);
  return 2;
}

static int mt_b2_body_velocity(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  b2Vec2 vel = body->GetLinearVelocity();

  lua_pushnumber(L, vel.x * physics->meter);
  lua_pushnumber(L, vel.y * physics->meter);
  return 2;
}

static int mt_b2_body_angle(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  lua_pushnumber(L, body->GetAngle());
  return 1;
}

static int mt_b2_body_linear_damping(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  lua_pushnumber(L, body->GetLinearDamping());
  return 1;
}

static int mt_b2_body_fixed_rotation(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  lua_pushboolean(L, body->IsFixedRotation());
  return 1;
}

static int mt_b2_body_apply_force(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  float x = luaL_checknumber(L, 2);
  float y = luaL_checknumber(L, 3);

  body->ApplyForceToCenter({x / physics->meter, y / physics->meter}, false);
  return 0;
}

static int mt_b2_body_apply_impulse(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  float x = luaL_checknumber(L, 2);
  float y = luaL_checknumber(L, 3);

  body->ApplyLinearImpulseToCenter({x / physics->meter, y / physics->meter},
                                   false);
  return 0;
}

static int mt_b2_body_set_position(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  float x = luaL_checknumber(L, 2);
  float y = luaL_checknumber(L, 3);

  body->SetTransform({x / physics->meter, y / physics->meter},
                     body->GetAngle());
  return 0;
}

static int mt_b2_body_set_velocity(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  float x = luaL_checknumber(L, 2);
  float y = luaL_checknumber(L, 3);

  body->SetLinearVelocity({x / physics->meter, y / physics->meter});
  return 0;
}

static int mt_b2_body_set_angle(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  float angle = luaL_checknumber(L, 2);

  body->SetTransform(body->GetPosition(), angle);
  return 0;
}

static int mt_b2_body_set_linear_damping(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  float damping = luaL_checknumber(L, 2);

  body->SetLinearDamping(damping);
  return 0;
}

static int mt_b2_body_set_fixed_rotation(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  bool fixed = lua_toboolean(L, 2);

  body->SetFixedRotation(fixed);
  return 0;
}

static int mt_b2_body_set_transform(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  float x = luaL_checknumber(L, 2);
  float y = luaL_checknumber(L, 3);
  float angle = luaL_checknumber(L, 4);

  body->SetTransform({x / physics->meter, y / physics->meter}, angle);
  return 0;
}

static int mt_b2_body_draw_fixtures(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  draw_fixtures_for_body(body, physics->meter);

  return 0;
}

static int mt_b2_body_udata(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;

  physics_push_userdata(L, body->GetUserData().pointer);
  return 1;
}

static int open_mt_b2_body(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_b2_body_gc},
      {"destroy", mt_b2_body_destroy},
      {"make_box_fixture", mt_b2_body_make_box_fixture},
      {"make_circle_fixture", mt_b2_body_make_circle_fixture},
      {"position", mt_b2_body_position},
      {"velocity", mt_b2_body_velocity},
      {"angle", mt_b2_body_angle},
      {"linear_damping", mt_b2_body_linear_damping},
      {"fixed_rotation", mt_b2_body_fixed_rotation},
      {"apply_force", mt_b2_body_apply_force},
      {"apply_impulse", mt_b2_body_apply_impulse},
      {"set_position", mt_b2_body_set_position},
      {"set_velocity", mt_b2_body_set_velocity},
      {"set_angle", mt_b2_body_set_angle},
      {"set_linear_damping", mt_b2_body_set_linear_damping},
      {"set_fixed_rotation", mt_b2_body_set_fixed_rotation},
      {"set_transform", mt_b2_body_set_transform},
      {"draw_fixtures", mt_b2_body_draw_fixtures},
      {"udata", mt_b2_body_udata},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_b2_body", reg);
  return 0;
}

// box2d world

static int mt_b2_world_gc(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
  physics_world_trash(L, physics);
  return 0;
}

static int mt_b2_world_step(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
  lua_Number dt = luaL_optnumber(L, 2, g_app->time.delta);
  lua_Integer vel_iters = luaL_optinteger(L, 3, 6);
  lua_Integer pos_iters = luaL_optinteger(L, 4, 2);

  physics->world->Step((float)dt, (i32)vel_iters, (i32)pos_iters);
  return 0;
}

static b2BodyDef b2_body_def(lua_State *L, i32 arg, Physics *physics) {
  lua_Number x = luax_number_field(L, arg, "x");
  lua_Number y = luax_number_field(L, arg, "y");
  lua_Number vx = luax_opt_number_field(L, arg, "vx", 0);
  lua_Number vy = luax_opt_number_field(L, arg, "vy", 0);
  lua_Number angle = luax_opt_number_field(L, arg, "angle", 0);
  lua_Number linear_damping =
      luax_opt_number_field(L, arg, "linear_damping", 0);
  bool fixed_rotation = luax_boolean_field(L, arg, "fixed_rotation");
  PhysicsUserData *pud = physics_userdata(L);

  b2BodyDef def = {};
  def.position.Set((float)x / physics->meter, (float)y / physics->meter);
  def.linearVelocity.Set((float)vx / physics->meter,
                         (float)vy / physics->meter);
  def.angle = angle;
  def.linearDamping = linear_damping;
  def.fixedRotation = fixed_rotation;
  def.userData.pointer = (u64)pud;
  return def;
}

static int b2_make_body(lua_State *L, b2BodyType type) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
  b2BodyDef body_def = b2_body_def(L, 2, physics);
  body_def.type = type;

  Physics p = physics_weak_copy(physics);
  p.body = physics->world->CreateBody(&body_def);

  luax_new_userdata(L, p, "mt_b2_body");
  return 1;
}

static int mt_b2_world_make_static_body(lua_State *L) {
  return b2_make_body(L, b2_staticBody);
}

static int mt_b2_world_make_kinematic_body(lua_State *L) {
  return b2_make_body(L, b2_kinematicBody);
}

static int mt_b2_world_make_dynamic_body(lua_State *L) {
  return b2_make_body(L, b2_dynamicBody);
}

static int mt_b2_world_begin_contact(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
  if (lua_type(L, 2) != LUA_TFUNCTION) {
    return luaL_error(L, "expected argument 2 to be a function");
  }

  physics_world_begin_contact(L, physics, 2);
  return 0;
}

static int mt_b2_world_end_contact(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
  if (lua_type(L, 2) != LUA_TFUNCTION) {
    return luaL_error(L, "expected argument 2 to be a function");
  }

  physics_world_end_contact(L, physics, 2);
  return 0;
}

static int open_mt_b2_world(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_b2_world_gc},
      {"destroy", mt_b2_world_gc},
      {"step", mt_b2_world_step},
      {"make_static_body", mt_b2_world_make_static_body},
      {"make_kinematic_body", mt_b2_world_make_kinematic_body},
      {"make_dynamic_body", mt_b2_world_make_dynamic_body},
      {"begin_contact", mt_b2_world_begin_contact},
      {"end_contact", mt_b2_world_end_contact},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_b2_world", reg);
  return 0;
}

// mt_mu_container

static int mt_mu_container_rect(lua_State *L) {
  mu_Container *container =
      *(mu_Container **)luaL_checkudata(L, 1, "mt_mu_container");
  lua_mu_rect_push(L, container->rect);
  return 1;
}

static int mt_mu_container_set_rect(lua_State *L) {
  mu_Container *container =
      *(mu_Container **)luaL_checkudata(L, 1, "mt_mu_container");
  container->rect = lua_mu_check_rect(L, 2);
  return 0;
}

static int mt_mu_container_body(lua_State *L) {
  mu_Container *container =
      *(mu_Container **)luaL_checkudata(L, 1, "mt_mu_container");
  lua_mu_rect_push(L, container->body);
  return 1;
}

static int mt_mu_container_content_size(lua_State *L) {
  mu_Container *container =
      *(mu_Container **)luaL_checkudata(L, 1, "mt_mu_container");
  lua_pushinteger(L, container->content_size.x);
  lua_pushinteger(L, container->content_size.y);
  return 2;
}

static int mt_mu_container_scroll(lua_State *L) {
  mu_Container *container =
      *(mu_Container **)luaL_checkudata(L, 1, "mt_mu_container");
  lua_pushinteger(L, container->scroll.x);
  lua_pushinteger(L, container->scroll.y);
  return 2;
}

static int mt_mu_container_set_scroll(lua_State *L) {
  mu_Container *container =
      *(mu_Container **)luaL_checkudata(L, 1, "mt_mu_container");
  container->scroll.x = luaL_checknumber(L, 2);
  container->scroll.y = luaL_checknumber(L, 3);
  return 0;
}

static int mt_mu_container_zindex(lua_State *L) {
  mu_Container *container =
      *(mu_Container **)luaL_checkudata(L, 1, "mt_mu_container");
  lua_pushinteger(L, container->zindex);
  return 1;
}

static int mt_mu_container_open(lua_State *L) {
  mu_Container *container =
      *(mu_Container **)luaL_checkudata(L, 1, "mt_mu_container");
  lua_pushboolean(L, container->open);
  return 1;
}

static int open_mt_mu_container(lua_State *L) {
  luaL_Reg reg[] = {
      {"rect", mt_mu_container_rect},
      {"set_rect", mt_mu_container_set_rect},
      {"body", mt_mu_container_body},
      {"content_size", mt_mu_container_content_size},
      {"scroll", mt_mu_container_scroll},
      {"set_scroll", mt_mu_container_set_scroll},
      {"zindex", mt_mu_container_zindex},
      {"open", mt_mu_container_open},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_mu_container", reg);
  return 0;
}

static int mt_mu_style_size(lua_State *L) {
  mu_Style *style = *(mu_Style **)luaL_checkudata(L, 1, "mt_mu_style");
  lua_pushinteger(L, style->size.x);
  lua_pushinteger(L, style->size.y);
  return 2;
}

static int mt_mu_style_set_size(lua_State *L) {
  mu_Style *style = *(mu_Style **)luaL_checkudata(L, 1, "mt_mu_style");
  style->size.x = luaL_checknumber(L, 2);
  style->size.y = luaL_checknumber(L, 3);
  return 0;
}

#define MT_MU_STYLE_GETSET(name)                                               \
  static int mt_mu_style_##name(lua_State *L) {                                \
    mu_Style *style = *(mu_Style **)luaL_checkudata(L, 1, "mt_mu_style");      \
    lua_pushinteger(L, style->name);                                           \
    return 1;                                                                  \
  }                                                                            \
  static int mt_mu_style_set_##name(lua_State *L) {                            \
    mu_Style *style = *(mu_Style **)luaL_checkudata(L, 1, "mt_mu_style");      \
    style->name = luaL_checknumber(L, 2);                                      \
    return 0;                                                                  \
  }

MT_MU_STYLE_GETSET(padding);
MT_MU_STYLE_GETSET(spacing);
MT_MU_STYLE_GETSET(indent);
MT_MU_STYLE_GETSET(title_height);
MT_MU_STYLE_GETSET(scrollbar_size);
MT_MU_STYLE_GETSET(thumb_size);

static int mt_mu_style_color(lua_State *L) {
  mu_Style *style = *(mu_Style **)luaL_checkudata(L, 1, "mt_mu_style");
  lua_Integer colorid = luaL_checkinteger(L, 2);
  if (colorid < 0 || colorid >= MU_COLOR_MAX) {
    return luaL_error(L, "color id out of range");
  }

  lua_createtable(L, 0, 4);
  luax_set_number_field(L, "r", style->colors[colorid].r);
  luax_set_number_field(L, "g", style->colors[colorid].g);
  luax_set_number_field(L, "b", style->colors[colorid].b);
  luax_set_number_field(L, "a", style->colors[colorid].a);
  return 1;
}

static int mt_mu_style_set_color(lua_State *L) {
  mu_Style *style = *(mu_Style **)luaL_checkudata(L, 1, "mt_mu_style");
  lua_Integer colorid = luaL_checkinteger(L, 2);
  mu_Color color = lua_mu_check_color(L, 3);

  if (colorid < 0 || colorid >= MU_COLOR_MAX) {
    return luaL_error(L, "color id out of range");
  }

  style->colors[colorid] = color;
  return 0;
}

static int open_mt_mu_style(lua_State *L) {
  luaL_Reg reg[] = {
      {"size", mt_mu_style_size},
      {"set_size", mt_mu_style_set_size},
      {"padding", mt_mu_style_padding},
      {"set_padding", mt_mu_style_set_padding},
      {"spacing", mt_mu_style_spacing},
      {"set_spacing", mt_mu_style_set_spacing},
      {"indent", mt_mu_style_indent},
      {"set_indent", mt_mu_style_set_indent},
      {"title_height", mt_mu_style_title_height},
      {"set_title_height", mt_mu_style_set_title_height},
      {"scrollbar_size", mt_mu_style_scrollbar_size},
      {"set_scrollbar_size", mt_mu_style_set_scrollbar_size},
      {"thumb_size", mt_mu_style_thumb_size},
      {"set_thumb_size", mt_mu_style_set_thumb_size},
      {"color", mt_mu_style_color},
      {"set_color", mt_mu_style_set_color},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_mu_style", reg);
  return 0;
}

// mt_mu_ref

static int mt_mu_ref_gc(lua_State *L) {
  MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_mu_ref");
  mem_free(ref);
  return 0;
}

static int mt_mu_ref_get(lua_State *L) {
  MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_mu_ref");

  switch (ref->kind) {
  case MUIRefKind_Boolean: lua_pushboolean(L, ref->boolean); return 1;
  case MUIRefKind_Real: lua_pushnumber(L, ref->real); return 1;
  case MUIRefKind_String: lua_pushstring(L, ref->string); return 1;
  case MUIRefKind_Nil:
  default: return 0;
  }
}

static int mt_mu_ref_set(lua_State *L) {
  MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_mu_ref");
  lua_mu_set_ref(L, ref, 2);
  return 0;
}

static int open_mt_mu_ref(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_mu_ref_gc},
      {"get", mt_mu_ref_get},
      {"set", mt_mu_ref_set},
      {nullptr, nullptr},
  };
  luax_new_class(L, "mt_mu_ref", reg);
  return 0;
}

// microui api

static int mui_set_focus(lua_State *L) {
  lua_Integer id = luaL_checkinteger(L, 1);
  mu_set_focus(microui_ctx(), (mu_Id)id);
  return 0;
}

static int mui_get_id(lua_State *L) {
  String name = luax_check_string(L, 1);
  mu_Id id = mu_get_id(microui_ctx(), name.data, name.len);
  lua_pushinteger(L, (lua_Integer)id);
  return 1;
}

static int mui_push_id(lua_State *L) {
  String name = luax_check_string(L, 1);
  mu_push_id(microui_ctx(), name.data, name.len);
  return 0;
}

static int mui_pop_id(lua_State *L) {
  mu_pop_id(microui_ctx());
  return 0;
}

static int mui_push_clip_rect(lua_State *L) {
  mu_Rect rect = lua_mu_check_rect(L, 1);
  mu_push_clip_rect(microui_ctx(), rect);
  return 0;
}

static int mui_pop_clip_rect(lua_State *L) {
  mu_pop_clip_rect(microui_ctx());
  return 0;
}

static int mui_get_clip_rect(lua_State *L) {
  mu_Rect rect = mu_get_clip_rect(microui_ctx());
  lua_mu_rect_push(L, rect);
  return 1;
}

static int mui_check_clip(lua_State *L) {
  mu_Rect rect = lua_mu_check_rect(L, 1);

  i32 clip = mu_check_clip(microui_ctx(), rect);
  lua_pushinteger(L, clip);
  return 1;
}

static int mui_get_current_container(lua_State *L) {
  mu_Container *container = mu_get_current_container(microui_ctx());
  luax_ptr_userdata(L, container, "mt_mu_container");
  return 1;
}

static int mui_get_container(lua_State *L) {
  String name = luax_check_string(L, 1);
  mu_Container *container = mu_get_container(microui_ctx(), name.data);
  luax_ptr_userdata(L, container, "mt_mu_container");
  return 1;
}

static int mui_bring_to_front(lua_State *L) {
  mu_Container *container =
      *(mu_Container **)luaL_checkudata(L, 1, "mt_mu_container");
  mu_bring_to_front(microui_ctx(), container);
  return 0;
}

static int mui_set_clip(lua_State *L) {
  mu_Rect rect = lua_mu_check_rect(L, 1);
  mu_set_clip(microui_ctx(), rect);
  return 0;
}

static int mui_draw_rect(lua_State *L) {
  mu_Rect rect = lua_mu_check_rect(L, 1);
  mu_Color color = lua_mu_check_color(L, 2);
  mu_draw_rect(microui_ctx(), rect, color);
  return 0;
}

static int mui_draw_box(lua_State *L) {
  mu_Rect rect = lua_mu_check_rect(L, 1);
  mu_Color color = lua_mu_check_color(L, 2);
  mu_draw_box(microui_ctx(), rect, color);
  return 0;
}

static int mui_draw_text(lua_State *L) {
  String str = luax_check_string(L, 1);
  lua_Number x = luaL_checknumber(L, 2);
  lua_Number y = luaL_checknumber(L, 3);
  mu_Color color = lua_mu_check_color(L, 4);

  mu_Vec2 pos = {(int)x, (int)y};
  mu_draw_text(microui_ctx(), nullptr, str.data, str.len, pos, color);
  return 0;
}

static int mui_draw_icon(lua_State *L) {
  lua_Integer id = luaL_checkinteger(L, 1);
  mu_Rect rect = lua_mu_check_rect(L, 2);
  mu_Color color = lua_mu_check_color(L, 3);

  mu_draw_icon(microui_ctx(), id, rect, color);
  return 0;
}

static int mui_layout_row(lua_State *L) {
  lua_Number height = luaL_checknumber(L, 2);

  i32 widths[MU_MAX_WIDTHS] = {};

  lua_Integer n = luax_len(L, 1);
  if (n > MU_MAX_WIDTHS) {
    n = MU_MAX_WIDTHS;
  }

  for (i32 i = 0; i < n; i++) {
    luax_geti(L, 1, i + 1);
    widths[i] = luaL_checknumber(L, -1);
    lua_pop(L, 1);
  }

  mu_layout_row(microui_ctx(), n, widths, height);
  return 0;
}

static int mui_layout_width(lua_State *L) {
  lua_Number width = luaL_checknumber(L, 1);
  mu_layout_width(microui_ctx(), width);
  return 0;
}

static int mui_layout_height(lua_State *L) {
  lua_Number height = luaL_checknumber(L, 1);
  mu_layout_height(microui_ctx(), height);
  return 0;
}

static int mui_layout_begin_column(lua_State *L) {
  mu_layout_begin_column(microui_ctx());
  return 0;
}

static int mui_layout_end_column(lua_State *L) {
  mu_layout_end_column(microui_ctx());
  return 0;
}

static int mui_layout_set_next(lua_State *L) {
  mu_Rect rect = lua_mu_check_rect(L, 1);
  bool relative = lua_toboolean(L, 2);
  mu_layout_set_next(microui_ctx(), rect, relative);
  return 0;
}

static int mui_layout_next(lua_State *L) {
  mu_Rect rect = mu_layout_next(microui_ctx());
  lua_mu_rect_push(L, rect);
  return 1;
}

static int mui_draw_control_frame(lua_State *L) {
  lua_Integer id = luaL_checkinteger(L, 1);
  mu_Rect rect = lua_mu_check_rect(L, 2);
  lua_Integer colorid = luaL_checkinteger(L, 3);
  lua_Integer opt = luaL_checkinteger(L, 4);
  mu_draw_control_frame(microui_ctx(), id, rect, colorid, opt);
  return 0;
}

static int mui_draw_control_text(lua_State *L) {
  String str = luax_check_string(L, 1);
  mu_Rect rect = lua_mu_check_rect(L, 2);
  lua_Integer colorid = luaL_checkinteger(L, 3);
  lua_Integer opt = luaL_checkinteger(L, 4);
  mu_draw_control_text(microui_ctx(), str.data, rect, colorid, opt);
  return 0;
}

static int mui_mouse_over(lua_State *L) {
  mu_Rect rect = lua_mu_check_rect(L, 1);
  int res = mu_mouse_over(microui_ctx(), rect);
  lua_pushboolean(L, res);
  return 1;
}

static int mui_update_control(lua_State *L) {
  lua_Integer id = luaL_checkinteger(L, 1);
  mu_Rect rect = lua_mu_check_rect(L, 2);
  lua_Integer opt = luaL_checkinteger(L, 3);
  mu_update_control(microui_ctx(), id, rect, opt);
  return 0;
}

static int mui_text(lua_State *L) {
  String text = luax_check_string(L, 1);
  mu_text(microui_ctx(), text.data);
  return 0;
}

static int mui_label(lua_State *L) {
  String text = luax_check_string(L, 1);
  mu_label(microui_ctx(), text.data);
  return 0;
}

static int mui_button(lua_State *L) {
  String text = luax_check_string(L, 1);
  lua_Integer icon = luaL_optinteger(L, 2, 0);
  lua_Integer opt = luaL_optinteger(L, 3, MU_OPT_ALIGNCENTER);
  i32 res = mu_button_ex(microui_ctx(), text.data, icon, opt);
  lua_pushboolean(L, res);
  return 1;
}

static int mui_checkbox(lua_State *L) {
  String text = luax_check_string(L, 1);
  MUIRef *ref = lua_mu_check_ref(L, 2, MUIRefKind_Boolean);

  i32 res = mu_checkbox(microui_ctx(), text.data, &ref->boolean);
  lua_pushinteger(L, res);
  return 1;
}

static int mui_textbox_raw(lua_State *L) {
  MUIRef *ref = lua_mu_check_ref(L, 1, MUIRefKind_String);
  lua_Integer id = luaL_checkinteger(L, 2);
  mu_Rect rect = lua_mu_check_rect(L, 3);
  i32 opt = luaL_optinteger(L, 4, 0);

  i32 res = mu_textbox_raw(microui_ctx(), ref->string, array_size(ref->string),
                           id, rect, opt);
  lua_pushinteger(L, res);
  return 1;
}

static int mui_textbox(lua_State *L) {
  MUIRef *ref = lua_mu_check_ref(L, 1, MUIRefKind_String);
  i32 opt = luaL_optinteger(L, 2, 0);

  i32 res =
      mu_textbox_ex(microui_ctx(), ref->string, array_size(ref->string), opt);
  lua_pushinteger(L, res);
  return 1;
}

static int mui_slider(lua_State *L) {
  MUIRef *ref = lua_mu_check_ref(L, 1, MUIRefKind_Real);
  mu_Real low = (mu_Real)luaL_checknumber(L, 2);
  mu_Real high = (mu_Real)luaL_checknumber(L, 3);
  mu_Real step = (mu_Real)luaL_optnumber(L, 4, 0);
  String fmt = luax_opt_string(L, 5, MU_SLIDER_FMT);
  i32 opt = luaL_optinteger(L, 6, MU_OPT_ALIGNCENTER);

  i32 res =
      mu_slider_ex(microui_ctx(), &ref->real, low, high, step, fmt.data, opt);
  lua_pushinteger(L, res);
  return 1;
}

static int mui_number(lua_State *L) {
  MUIRef *ref = lua_mu_check_ref(L, 1, MUIRefKind_Real);
  mu_Real step = (mu_Real)luaL_checknumber(L, 2);
  String fmt = luax_opt_string(L, 3, MU_SLIDER_FMT);
  i32 opt = luaL_optinteger(L, 4, MU_OPT_ALIGNCENTER);

  i32 res = mu_number_ex(microui_ctx(), &ref->real, step, fmt.data, opt);
  lua_pushinteger(L, res);
  return 1;
}

static int mui_header(lua_State *L) {
  String text = luax_check_string(L, 1);
  lua_Integer opt = luaL_optinteger(L, 2, 0);
  i32 res = mu_header_ex(microui_ctx(), text.data, opt);
  lua_pushboolean(L, res);
  return 1;
}

static int mui_begin_treenode(lua_State *L) {
  String label = luax_check_string(L, 1);
  lua_Integer opt = luaL_optinteger(L, 2, 0);

  i32 res = mu_begin_treenode_ex(microui_ctx(), label.data, opt);
  lua_pushboolean(L, res);
  return 1;
}

static int mui_end_treenode(lua_State *L) {
  mu_end_treenode(microui_ctx());
  return 0;
}

static int mui_begin_window(lua_State *L) {
  String title = luax_check_string(L, 1);
  mu_Rect rect = lua_mu_check_rect(L, 2);
  lua_Integer opt = luaL_optinteger(L, 3, 0);

  i32 res = mu_begin_window_ex(microui_ctx(), title.data, rect, opt);
  lua_pushboolean(L, res);
  return 1;
}

static int mui_end_window(lua_State *L) {
  mu_end_window(microui_ctx());
  return 0;
}

static int mui_open_popup(lua_State *L) {
  String name = luax_check_string(L, 1);
  mu_open_popup(microui_ctx(), name.data);
  return 0;
}

static int mui_begin_popup(lua_State *L) {
  String name = luax_check_string(L, 1);
  i32 res = mu_begin_popup(microui_ctx(), name.data);
  lua_pushboolean(L, res);
  return 1;
}

static int mui_end_popup(lua_State *L) {
  mu_end_popup(microui_ctx());
  return 0;
}

static int mui_begin_panel(lua_State *L) {
  String name = luax_check_string(L, 1);
  lua_Integer opt = luaL_optinteger(L, 2, 0);
  mu_begin_panel_ex(microui_ctx(), name.data, opt);
  return 0;
}

static int mui_end_panel(lua_State *L) {
  mu_end_panel(microui_ctx());
  return 0;
}

static int mui_get_hover(lua_State *L) {
  lua_pushinteger(L, microui_ctx()->hover);
  return 1;
}

static int mui_get_focus(lua_State *L) {
  lua_pushinteger(L, microui_ctx()->focus);
  return 1;
}

static int mui_get_last_id(lua_State *L) {
  lua_pushinteger(L, microui_ctx()->last_id);
  return 1;
}

static int mui_get_style(lua_State *L) {
  luax_ptr_userdata(L, microui_ctx()->style, "mt_mu_style");
  return 1;
}

static int mui_rect(lua_State *L) {
  lua_createtable(L, 0, 4);
  luax_set_int_field(L, "x", luaL_checknumber(L, 1));
  luax_set_int_field(L, "y", luaL_checknumber(L, 2));
  luax_set_int_field(L, "w", luaL_checknumber(L, 3));
  luax_set_int_field(L, "h", luaL_checknumber(L, 4));
  return 1;
}

static int mui_color(lua_State *L) {
  lua_createtable(L, 0, 4);
  luax_set_int_field(L, "r", luaL_checknumber(L, 1));
  luax_set_int_field(L, "g", luaL_checknumber(L, 2));
  luax_set_int_field(L, "b", luaL_checknumber(L, 3));
  luax_set_int_field(L, "a", luaL_checknumber(L, 4));
  return 1;
}

static int mui_ref(lua_State *L) {
  MUIRef *ref = (MUIRef *)mem_alloc(sizeof(MUIRef));
  lua_mu_set_ref(L, ref, 1);
  luax_ptr_userdata(L, ref, "mt_mu_ref");
  return 1;
}

static int open_microui(lua_State *L) {
  luaL_Reg reg[] = {
      {"set_focus", mui_set_focus},
      {"get_id", mui_get_id},
      {"push_id", mui_push_id},
      {"pop_id", mui_pop_id},
      {"push_clip_rect", mui_push_clip_rect},
      {"pop_clip_rect", mui_pop_clip_rect},
      {"get_clip_rect", mui_get_clip_rect},
      {"check_clip", mui_check_clip},
      {"get_current_container", mui_get_current_container},
      {"get_container", mui_get_container},
      {"bring_to_front", mui_bring_to_front},

      {"set_clip", mui_set_clip},
      {"draw_rect", mui_draw_rect},
      {"draw_box", mui_draw_box},
      {"draw_text", mui_draw_text},
      {"draw_icon", mui_draw_icon},

      {"layout_row", mui_layout_row},
      {"layout_width", mui_layout_width},
      {"layout_height", mui_layout_height},
      {"layout_begin_column", mui_layout_begin_column},
      {"layout_end_column", mui_layout_end_column},
      {"layout_set_next", mui_layout_set_next},
      {"layout_next", mui_layout_next},

      {"draw_control_frame", mui_draw_control_frame},
      {"draw_control_text", mui_draw_control_text},
      {"mouse_over", mui_mouse_over},
      {"update_control", mui_update_control},

      {"text", mui_text},
      {"label", mui_label},
      {"button", mui_button},
      {"checkbox", mui_checkbox},
      {"textbox_raw", mui_textbox_raw},
      {"textbox", mui_textbox},
      {"slider", mui_slider},
      {"number", mui_number},
      {"header", mui_header},
      {"begin_treenode", mui_begin_treenode},
      {"end_treenode", mui_end_treenode},
      {"begin_window", mui_begin_window},
      {"end_window", mui_end_window},
      {"open_popup", mui_open_popup},
      {"begin_popup", mui_begin_popup},
      {"end_popup", mui_end_popup},
      {"begin_panel", mui_begin_panel},
      {"end_panel", mui_end_panel},

      // access
      {"get_hover", mui_get_hover},
      {"get_focus", mui_get_focus},
      {"get_last_id", mui_get_last_id},
      {"get_style", mui_get_style},

      // utility
      {"rect", mui_rect},
      {"color", mui_color},
      {"ref", mui_ref},
      {nullptr, nullptr},
  };

  luaL_newlib(L, reg);

  luax_set_string_field(L, "VERSION", MU_VERSION);

  luax_set_int_field(L, "COMMANDLIST_SIZE", MU_COMMANDLIST_SIZE);
  luax_set_int_field(L, "ROOTLIST_SIZE", MU_ROOTLIST_SIZE);
  luax_set_int_field(L, "CONTAINERSTACK_SIZE", MU_CONTAINERSTACK_SIZE);
  luax_set_int_field(L, "CLIPSTACK_SIZE", MU_CLIPSTACK_SIZE);
  luax_set_int_field(L, "IDSTACK_SIZE", MU_IDSTACK_SIZE);
  luax_set_int_field(L, "LAYOUTSTACK_SIZE", MU_LAYOUTSTACK_SIZE);
  luax_set_int_field(L, "CONTAINERPOOL_SIZE", MU_CONTAINERPOOL_SIZE);
  luax_set_int_field(L, "TREENODEPOOL_SIZE", MU_TREENODEPOOL_SIZE);
  luax_set_int_field(L, "MAX_WIDTHS", MU_MAX_WIDTHS);
  luax_set_string_field(L, "REAL_FMT", MU_REAL_FMT);
  luax_set_string_field(L, "SLIDER_FMT", MU_SLIDER_FMT);
  luax_set_int_field(L, "MAX_FMT", MU_MAX_FMT);

  luax_set_int_field(L, "CLIP_PART", MU_CLIP_PART);
  luax_set_int_field(L, "CLIP_ALL", MU_CLIP_ALL);

  luax_set_int_field(L, "COMMAND_JUMP", MU_COMMAND_JUMP);
  luax_set_int_field(L, "COMMAND_CLIP", MU_COMMAND_CLIP);
  luax_set_int_field(L, "COMMAND_RECT", MU_COMMAND_RECT);
  luax_set_int_field(L, "COMMAND_TEXT", MU_COMMAND_TEXT);
  luax_set_int_field(L, "COMMAND_ICON", MU_COMMAND_ICON);

  luax_set_int_field(L, "COLOR_TEXT", MU_COLOR_TEXT);
  luax_set_int_field(L, "COLOR_BORDER", MU_COLOR_BORDER);
  luax_set_int_field(L, "COLOR_WINDOWBG", MU_COLOR_WINDOWBG);
  luax_set_int_field(L, "COLOR_TITLEBG", MU_COLOR_TITLEBG);
  luax_set_int_field(L, "COLOR_TITLETEXT", MU_COLOR_TITLETEXT);
  luax_set_int_field(L, "COLOR_PANELBG", MU_COLOR_PANELBG);
  luax_set_int_field(L, "COLOR_BUTTON", MU_COLOR_BUTTON);
  luax_set_int_field(L, "COLOR_BUTTONHOVER", MU_COLOR_BUTTONHOVER);
  luax_set_int_field(L, "COLOR_BUTTONFOCUS", MU_COLOR_BUTTONFOCUS);
  luax_set_int_field(L, "COLOR_BASE", MU_COLOR_BASE);
  luax_set_int_field(L, "COLOR_BASEHOVER", MU_COLOR_BASEHOVER);
  luax_set_int_field(L, "COLOR_BASEFOCUS", MU_COLOR_BASEFOCUS);
  luax_set_int_field(L, "COLOR_SCROLLBASE", MU_COLOR_SCROLLBASE);
  luax_set_int_field(L, "COLOR_SCROLLTHUMB", MU_COLOR_SCROLLTHUMB);

  luax_set_int_field(L, "ICON_CLOSE", MU_ICON_CLOSE);
  luax_set_int_field(L, "ICON_CHECK", MU_ICON_CHECK);
  luax_set_int_field(L, "ICON_COLLAPSED", MU_ICON_COLLAPSED);
  luax_set_int_field(L, "ICON_EXPANDED", MU_ICON_EXPANDED);

  luax_set_int_field(L, "RES_ACTIVE", MU_RES_ACTIVE);
  luax_set_int_field(L, "RES_SUBMIT", MU_RES_SUBMIT);
  luax_set_int_field(L, "RES_CHANGE", MU_RES_CHANGE);

  luax_set_int_field(L, "OPT_ALIGNCENTER", MU_OPT_ALIGNCENTER);
  luax_set_int_field(L, "OPT_ALIGNRIGHT", MU_OPT_ALIGNRIGHT);
  luax_set_int_field(L, "OPT_NOINTERACT", MU_OPT_NOINTERACT);
  luax_set_int_field(L, "OPT_NOFRAME", MU_OPT_NOFRAME);
  luax_set_int_field(L, "OPT_NORESIZE", MU_OPT_NORESIZE);
  luax_set_int_field(L, "OPT_NOSCROLL", MU_OPT_NOSCROLL);
  luax_set_int_field(L, "OPT_NOCLOSE", MU_OPT_NOCLOSE);
  luax_set_int_field(L, "OPT_NOTITLE", MU_OPT_NOTITLE);
  luax_set_int_field(L, "OPT_HOLDFOCUS", MU_OPT_HOLDFOCUS);
  luax_set_int_field(L, "OPT_AUTOSIZE", MU_OPT_AUTOSIZE);
  luax_set_int_field(L, "OPT_POPUP", MU_OPT_POPUP);
  luax_set_int_field(L, "OPT_CLOSED", MU_OPT_CLOSED);
  luax_set_int_field(L, "OPT_EXPANDED", MU_OPT_EXPANDED);

  return 1;
}

// spry api

static int spry_registry_load(lua_State *L) {
  if (lua_gettop(L) < 2) {
    luaL_error(L, "expected a name and a value");
  }

  String name = luax_check_string(L, 1);

  // registry._LOADED
  lua_pushliteral(L, LUA_LOADED_TABLE);
  lua_gettable(L, LUA_REGISTRYINDEX);

  if (lua_type(L, 2) == LUA_TNIL) {
    lua_pushboolean(L, true);
  } else {
    lua_pushvalue(L, 2);
  }
  // _LOADED[name] = value
  lua_setfield(L, -2, name.data);

  lua_pushvalue(L, 2);
  return 1;
}

static int spry_require_lua_script(lua_State *L) {
  PROFILE_FUNC();

  String path = luax_check_string(L, 1);

  Asset asset = {};
  bool ok = asset_load(AssetKind_LuaRef, path, &asset);
  if (!ok) {
    return 0;
  }

  lua_rawgeti(L, LUA_REGISTRYINDEX, asset.lua_ref);
  return 1;
}

static int spry_version(lua_State *L) {
  lua_pushstring(L, SPRY_VERSION);
  return 1;
}

static int spry_quit(lua_State *L) {
  (void)L;
  sapp_request_quit();
  return 0;
}

static int spry_platform(lua_State *L) {
#if defined(IS_HTML5)
  lua_pushliteral(L, "html5");
#elif defined(IS_WIN32)
  lua_pushliteral(L, "windows");
#elif defined(IS_LINUX)
  lua_pushliteral(L, "linux");
#endif
  return 1;
}

static int spry_program_path(lua_State *L) {
  String path = os_program_path();
  lua_pushlstring(L, path.data, path.len);
  return 1;
}

static int spry_dt(lua_State *L) {
  lua_pushnumber(L, g_app->time.delta);
  return 1;
}

static int spry_fullscreen(lua_State *L) {
  lua_pushboolean(L, sapp_is_fullscreen());
  return 1;
}

static int spry_toggle_fullscreen(lua_State *L) {
  sapp_toggle_fullscreen();
  return 0;
}

static int spry_window_width(lua_State *L) {
  float width = sapp_widthf();
  lua_pushnumber(L, width);
  return 1;
}

static int spry_window_height(lua_State *L) {
  float height = sapp_heightf();
  lua_pushnumber(L, height);
  return 1;
}

static int spry_time(lua_State *L) {
  lua_pushinteger(L, stm_now());
  return 1;
}

static int spry_difftime(lua_State *L) {
  lua_Integer t2 = luaL_checkinteger(L, 1);
  lua_Integer t1 = luaL_checkinteger(L, 2);

  lua_pushinteger(L, stm_diff(t2, t1));
  return 1;
}

static int spry_json_read(lua_State *L) {
  PROFILE_FUNC();

  String str = luax_check_string(L, 1);

  JSONDocument doc = {};
  json_parse(&doc, str);
  defer(json_trash(&doc));

  if (doc.error.len != 0) {
    lua_pushnil(L);
    lua_pushlstring(L, doc.error.data, doc.error.len);
    return 2;
  }

  {
    PROFILE_BLOCK("json to lua");
    json_to_lua(L, &doc.root);
  }
  return 1;
}

static int spry_json_write(lua_State *L) {
  PROFILE_FUNC();

  String contents = {};
  String err = {};
  lua_to_json_string(L, 1, &contents, &err);
  if (err.len != 0) {
    lua_pushnil(L);
    lua_pushlstring(L, err.data, err.len);
    return 2;
  }

  lua_pushlstring(L, contents.data, contents.len);
  mem_free(contents.data);
  return 1;
}

static i32 keyboard_lookup(String str) {
  switch (fnv1a(str)) {
  case "space"_hash: return 32;
  case "'"_hash: return 39;
  case ","_hash: return 44;
  case "-"_hash: return 45;
  case "."_hash: return 46;
  case "/"_hash: return 47;
  case "0"_hash: return 48;
  case "1"_hash: return 49;
  case "2"_hash: return 50;
  case "3"_hash: return 51;
  case "4"_hash: return 52;
  case "5"_hash: return 53;
  case "6"_hash: return 54;
  case "7"_hash: return 55;
  case "8"_hash: return 56;
  case "9"_hash: return 57;
  case ";"_hash: return 59;
  case "="_hash: return 61;
  case "a"_hash: return 65;
  case "b"_hash: return 66;
  case "c"_hash: return 67;
  case "d"_hash: return 68;
  case "e"_hash: return 69;
  case "f"_hash: return 70;
  case "g"_hash: return 71;
  case "h"_hash: return 72;
  case "i"_hash: return 73;
  case "j"_hash: return 74;
  case "k"_hash: return 75;
  case "l"_hash: return 76;
  case "m"_hash: return 77;
  case "n"_hash: return 78;
  case "o"_hash: return 79;
  case "p"_hash: return 80;
  case "q"_hash: return 81;
  case "r"_hash: return 82;
  case "s"_hash: return 83;
  case "t"_hash: return 84;
  case "u"_hash: return 85;
  case "v"_hash: return 86;
  case "w"_hash: return 87;
  case "x"_hash: return 88;
  case "y"_hash: return 89;
  case "z"_hash: return 90;
  case "["_hash: return 91;
  case "\\"_hash: return 92;
  case "]"_hash: return 93;
  case "`"_hash: return 96;
  case "world_1"_hash: return 161;
  case "world_2"_hash: return 162;
  case "esc"_hash: return 256;
  case "enter"_hash: return 257;
  case "tab"_hash: return 258;
  case "backspace"_hash: return 259;
  case "insert"_hash: return 260;
  case "delete"_hash: return 261;
  case "right"_hash: return 262;
  case "left"_hash: return 263;
  case "down"_hash: return 264;
  case "up"_hash: return 265;
  case "pg_up"_hash: return 266;
  case "pg_down"_hash: return 267;
  case "home"_hash: return 268;
  case "end"_hash: return 269;
  case "caps_lock"_hash: return 280;
  case "scroll_lock"_hash: return 281;
  case "num_lock"_hash: return 282;
  case "print_screen"_hash: return 283;
  case "pause"_hash: return 284;
  case "f1"_hash: return 290;
  case "f2"_hash: return 291;
  case "f3"_hash: return 292;
  case "f4"_hash: return 293;
  case "f5"_hash: return 294;
  case "f6"_hash: return 295;
  case "f7"_hash: return 296;
  case "f8"_hash: return 297;
  case "f9"_hash: return 298;
  case "f10"_hash: return 299;
  case "f11"_hash: return 300;
  case "f12"_hash: return 301;
  case "f13"_hash: return 302;
  case "f14"_hash: return 303;
  case "f15"_hash: return 304;
  case "f16"_hash: return 305;
  case "f17"_hash: return 306;
  case "f18"_hash: return 307;
  case "f19"_hash: return 308;
  case "f20"_hash: return 309;
  case "f21"_hash: return 310;
  case "f22"_hash: return 311;
  case "f23"_hash: return 312;
  case "f24"_hash: return 313;
  case "f25"_hash: return 314;
  case "kp0"_hash: return 320;
  case "kp1"_hash: return 321;
  case "kp2"_hash: return 322;
  case "kp3"_hash: return 323;
  case "kp4"_hash: return 324;
  case "kp5"_hash: return 325;
  case "kp6"_hash: return 326;
  case "kp7"_hash: return 327;
  case "kp8"_hash: return 328;
  case "kp9"_hash: return 329;
  case "kp."_hash: return 330;
  case "kp/"_hash: return 331;
  case "kp*"_hash: return 332;
  case "kp-"_hash: return 333;
  case "kp+"_hash: return 334;
  case "kp_enter"_hash: return 335;
  case "kp="_hash: return 336;
  case "lshift"_hash: return 340;
  case "lctrl"_hash: return 341;
  case "lalt"_hash: return 342;
  case "lsuper"_hash: return 343;
  case "rshift"_hash: return 344;
  case "rctrl"_hash: return 345;
  case "ralt"_hash: return 346;
  case "rsuper"_hash: return 347;
  case "menu"_hash: return 348;
  default: return 0;
  }
}

static int spry_key_down(lua_State *L) {
  String str = luax_check_string(L, 1);
  i32 key = keyboard_lookup(str);
  bool is_down = g_app->key_state[key];
  lua_pushboolean(L, is_down);
  return 1;
}

static int spry_key_release(lua_State *L) {
  String str = luax_check_string(L, 1);
  i32 key = keyboard_lookup(str);
  bool is_release = !g_app->key_state[key] && g_app->prev_key_state[key];
  lua_pushboolean(L, is_release);
  return 1;
}

static int spry_key_press(lua_State *L) {
  String str = luax_check_string(L, 1);
  i32 key = keyboard_lookup(str);
  bool is_press = g_app->key_state[key] && !g_app->prev_key_state[key];
  lua_pushboolean(L, is_press);
  return 1;
}

static int spry_mouse_down(lua_State *L) {
  lua_Integer n = luaL_checkinteger(L, 1);
  if (n >= 0 && n < array_size(g_app->mouse_state)) {
    lua_pushboolean(L, g_app->mouse_state[n]);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}

static int spry_mouse_release(lua_State *L) {
  lua_Integer n = luaL_checkinteger(L, 1);
  if (n >= 0 && n < array_size(g_app->mouse_state)) {
    bool is_release = !g_app->mouse_state[n] && g_app->prev_mouse_state[n];
    lua_pushboolean(L, is_release);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}

static int spry_mouse_click(lua_State *L) {
  lua_Integer n = luaL_checkinteger(L, 1);
  if (n >= 0 && n < array_size(g_app->mouse_state)) {
    bool is_click = g_app->mouse_state[n] && !g_app->prev_mouse_state[n];
    lua_pushboolean(L, is_click);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}

static int spry_mouse_pos(lua_State *L) {
  lua_pushnumber(L, g_app->mouse_x);
  lua_pushnumber(L, g_app->mouse_y);
  return 2;
}

static int spry_mouse_delta(lua_State *L) {
  lua_pushnumber(L, g_app->mouse_x - g_app->prev_mouse_x);
  lua_pushnumber(L, g_app->mouse_y - g_app->prev_mouse_y);
  return 2;
}

static int spry_show_mouse(lua_State *L) {
  bool show = lua_toboolean(L, 1);
  sapp_show_mouse(show);
  return 0;
}

static int spry_scroll_wheel(lua_State *L) {
  lua_pushnumber(L, g_app->scroll_x);
  lua_pushnumber(L, g_app->scroll_y);
  return 2;
}

static int spry_scissor_rect(lua_State *L) {
  lua_Number x = luaL_optnumber(L, 1, 0);
  lua_Number y = luaL_optnumber(L, 2, 0);
  lua_Number w = luaL_optnumber(L, 3, sapp_widthf());
  lua_Number h = luaL_optnumber(L, 4, sapp_heightf());

  sgl_scissor_rectf(x, y, w, h, true);
  return 0;
}

static int spry_push_matrix(lua_State *L) {
  bool ok = renderer_push_matrix();
  return ok ? 0 : luaL_error(L, "matrix stack is full");
  return 0;
}

static int spry_pop_matrix(lua_State *L) {
  bool ok = renderer_pop_matrix();
  return ok ? 0 : luaL_error(L, "matrix stack is full");
  return 0;
}

static int spry_translate(lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);

  renderer_translate((float)x, (float)y);
  return 0;
}

static int spry_rotate(lua_State *L) {
  lua_Number angle = luaL_checknumber(L, 1);

  renderer_rotate((float)angle);
  return 0;
}

static int spry_scale(lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);

  renderer_scale((float)x, (float)y);
  return 0;
}

static int spry_clear_color(lua_State *L) {
  lua_Number r = luaL_checknumber(L, 1);
  lua_Number g = luaL_checknumber(L, 2);
  lua_Number b = luaL_checknumber(L, 3);
  lua_Number a = luaL_checknumber(L, 4);

  float rgba[4] = {
      (float)r / 255.0f,
      (float)g / 255.0f,
      (float)b / 255.0f,
      (float)a / 255.0f,
  };
  renderer_set_clear_color(rgba);

  return 0;
}

static int spry_push_color(lua_State *L) {
  lua_Number r = luaL_checknumber(L, 1);
  lua_Number g = luaL_checknumber(L, 2);
  lua_Number b = luaL_checknumber(L, 3);
  lua_Number a = luaL_checknumber(L, 4);

  Color color = {};
  color.r = (u8)r;
  color.g = (u8)g;
  color.b = (u8)b;
  color.a = (u8)a;

  bool ok = renderer_push_color(color);
  return ok ? 0 : luaL_error(L, "color stack is full");
}

static int spry_pop_color(lua_State *L) {
  bool ok = renderer_pop_color();
  return ok ? 0 : luaL_error(L, "color stack can't be less than 1");
}

static int spry_default_font(lua_State *L) {
  if (g_app->default_font == nullptr) {
    g_app->default_font = (FontFamily *)mem_alloc(sizeof(FontFamily));
    font_load_default(g_app->default_font);
  }

  luax_ptr_userdata(L, g_app->default_font, "mt_font");
  return 1;
}

static int spry_draw_filled_rect(lua_State *L) {
  RectDescription rd = rect_description_args(L, 1);
  draw_filled_rect(&rd);
  return 0;
}

static int spry_draw_line_rect(lua_State *L) {
  RectDescription rd = rect_description_args(L, 1);
  draw_line_rect(&rd);
  return 0;
}

static int spry_draw_line_circle(lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);
  lua_Number radius = luaL_checknumber(L, 3);

  draw_line_circle(x, y, radius);
  return 0;
}

static int spry_draw_line(lua_State *L) {
  lua_Number x0 = luaL_checknumber(L, 1);
  lua_Number y0 = luaL_checknumber(L, 2);
  lua_Number x1 = luaL_checknumber(L, 3);
  lua_Number y1 = luaL_checknumber(L, 4);

  draw_line(x0, y0, x1, y1);
  return 0;
}

static int spry_set_master_volume(lua_State *L) {
  lua_Number vol = luaL_checknumber(L, 1);
  ma_engine_set_volume(&g_app->audio_engine, (float)vol);
  return 0;
}

static sg_filter str_to_filter_mode(lua_State *L, String s) {
  switch (fnv1a(s)) {
  case "nearest"_hash: return SG_FILTER_NEAREST; break;
  case "linear"_hash: return SG_FILTER_LINEAR; break;
  default:
    luax_string_oneof(L, {"nearest", "linear"}, s);
    return _SG_FILTER_DEFAULT;
  }
}

static sg_wrap str_to_wrap_mode(lua_State *L, String s) {
  switch (fnv1a(s)) {
  case "repeat"_hash: return SG_WRAP_REPEAT; break;
  case "mirroredrepeat"_hash: return SG_WRAP_MIRRORED_REPEAT; break;
  case "clamp"_hash: return SG_WRAP_CLAMP_TO_EDGE; break;
  default:
    luax_string_oneof(L, {"repeat", "mirroredrepeat", "clamp"}, s);
    return _SG_WRAP_DEFAULT;
  }
}

static int spry_make_sampler(lua_State *L) {
  String min_filter = luax_opt_string_field(L, 1, "min_filter", "nearest");
  String mag_filter = luax_opt_string_field(L, 1, "mag_filter", "nearest");
  String wrap_u = luax_opt_string_field(L, 1, "wrap_u", "repeat");
  String wrap_v = luax_opt_string_field(L, 1, "wrap_v", "repeat");

  sg_sampler_desc desc = {};
  desc.min_filter = str_to_filter_mode(L, min_filter);
  desc.mag_filter = str_to_filter_mode(L, mag_filter);
  desc.wrap_u = str_to_wrap_mode(L, wrap_u);
  desc.wrap_v = str_to_wrap_mode(L, wrap_v);

  sg_sampler sampler = sg_make_sampler(&desc);

  luax_new_userdata(L, sampler.id, "mt_sampler");
  return 1;
}

static int spry_default_sampler(lua_State *L) {
  u32 id = SG_INVALID_ID;
  luax_new_userdata(L, id, "mt_sampler");
  return 1;
}

static int spry_image_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  Asset asset = {};
  bool ok = asset_load(AssetKind_Image, str, &asset);
  if (!ok) {
    return 0;
  }

  luax_new_userdata(L, asset.hash, "mt_image");
  return 1;
}

static int spry_font_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  FontFamily *font = (FontFamily *)mem_alloc(sizeof(FontFamily));
  bool ok = font_load(font, str);
  if (!ok) {
    mem_free(font);
    return 0;
  }

  luax_ptr_userdata(L, font, "mt_font");
  return 1;
}

static int spry_sound_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  Sound *sound = sound_load(str);
  if (sound == nullptr) {
    return 0;
  }

  luax_ptr_userdata(L, sound, "mt_sound");
  return 1;
}

static int spry_sprite_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  Asset asset = {};
  bool ok = asset_load(AssetKind_Sprite, str, &asset);
  if (!ok) {
    return 0;
  }

  Sprite spr = {};
  spr.sprite = asset.hash;

  luax_new_userdata(L, spr, "mt_sprite");
  return 1;
}

static int spry_atlas_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  Atlas atlas = {};
  bool ok = atlas_load(&atlas, str);
  if (!ok) {
    return 0;
  }

  luax_new_userdata(L, atlas, "mt_atlas");
  return 1;
}

static int spry_tilemap_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  Asset asset = {};
  bool ok = asset_load(AssetKind_Tilemap, str, &asset);
  if (!ok) {
    return 0;
  }

  luax_new_userdata(L, asset.hash, "mt_tilemap");
  return 1;
}

static int spry_b2_world(lua_State *L) {
  lua_Number gx = luax_opt_number_field(L, 1, "gx", 0);
  lua_Number gy = luax_opt_number_field(L, 1, "gy", 9.81);
  lua_Number meter = luax_opt_number_field(L, 1, "meter", 16);

  b2Vec2 gravity = {(float)gx, (float)gy};

  Physics p = physics_world_make(L, gravity, meter);
  luax_new_userdata(L, p, "mt_b2_world");
  return 1;
}

static int open_spry(lua_State *L) {
  luaL_Reg reg[] = {
      // internal
      {"_registry_load", spry_registry_load},
      {"_require_lua_script", spry_require_lua_script},

      // core
      {"version", spry_version},
      {"quit", spry_quit},
      {"platform", spry_platform},
      {"program_path", spry_program_path},
      {"dt", spry_dt},
      {"fullscreen", spry_fullscreen},
      {"toggle_fullscreen", spry_toggle_fullscreen},
      {"window_width", spry_window_width},
      {"window_height", spry_window_height},
      {"time", spry_time},
      {"difftime", spry_difftime},
      {"json_read", spry_json_read},
      {"json_write", spry_json_write},

      // input
      {"key_down", spry_key_down},
      {"key_release", spry_key_release},
      {"key_press", spry_key_press},
      {"mouse_down", spry_mouse_down},
      {"mouse_release", spry_mouse_release},
      {"mouse_click", spry_mouse_click},
      {"mouse_pos", spry_mouse_pos},
      {"mouse_delta", spry_mouse_delta},
      {"show_mouse", spry_show_mouse},
      {"scroll_wheel", spry_scroll_wheel},

      // draw
      {"scissor_rect", spry_scissor_rect},
      {"push_matrix", spry_push_matrix},
      {"pop_matrix", spry_pop_matrix},
      {"translate", spry_translate},
      {"rotate", spry_rotate},
      {"scale", spry_scale},
      {"clear_color", spry_clear_color},
      {"push_color", spry_push_color},
      {"pop_color", spry_pop_color},
      {"default_font", spry_default_font},
      {"default_sampler", spry_default_sampler},
      {"draw_filled_rect", spry_draw_filled_rect},
      {"draw_line_rect", spry_draw_line_rect},
      {"draw_line_circle", spry_draw_line_circle},
      {"draw_line", spry_draw_line},

      // audio
      {"set_master_volume", spry_set_master_volume},

      // construct types
      {"make_sampler", spry_make_sampler},
      {"image_load", spry_image_load},
      {"font_load", spry_font_load},
      {"sound_load", spry_sound_load},
      {"sprite_load", spry_sprite_load},
      {"atlas_load", spry_atlas_load},
      {"tilemap_load", spry_tilemap_load},
      {"b2_world", spry_b2_world},
      {nullptr, nullptr},
  };

  luaL_newlib(L, reg);
  return 1;
}

void open_spry_api(lua_State *L) {
  lua_CFunction mt_funcs[] = {
      open_mt_sampler,  open_mt_image,    open_mt_font,
      open_mt_sound,    open_mt_sprite,   open_mt_atlas_image,
      open_mt_atlas,    open_mt_tilemap,  open_mt_b2_fixture,
      open_mt_b2_body,  open_mt_b2_world, open_mt_mu_container,
      open_mt_mu_style, open_mt_mu_ref,
  };

  for (u32 i = 0; i < array_size(mt_funcs); i++) {
    mt_funcs[i](L);
  }

  luaL_requiref(L, "spry", open_spry, 1);

  open_microui(L);
  lua_setfield(L, -2, "microui");

  lua_pop(L, 1);
}

static void package_preload(lua_State *L, const char *name,
                            lua_CFunction function) {
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "preload");
  lua_pushcfunction(L, function);
  lua_setfield(L, -2, name);
  lua_pop(L, 2);
}

#ifdef IS_HTML5
void open_luasocket(lua_State *L) {}
#else
#define LUAOPEN_EMBED_DATA(func, name, compressed_data, compressed_size)       \
  static int func(lua_State *L) {                                              \
    i32 top = lua_gettop(L);                                                   \
                                                                               \
    String contents = stb_decompress_data(compressed_data, compressed_size);   \
    defer(mem_free(contents.data));                                            \
                                                                               \
    if (luaL_loadbuffer(L, contents.data, contents.len, name) != LUA_OK) {     \
      luaL_error(L, "%s", lua_tostring(L, -1));                                \
      return 0;                                                                \
    }                                                                          \
                                                                               \
    if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {                           \
      luaL_error(L, "%s", lua_tostring(L, -1));                                \
      return 0;                                                                \
    }                                                                          \
                                                                               \
    return lua_gettop(L) - top;                                                \
  }

LUAOPEN_EMBED_DATA(open_embed_ltn12, "ltn12.lua", ltn12_compressed_data,
                   ltn12_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_mbox, "mbox.lua", mbox_compressed_data,
                   mbox_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_mime, "mime.lua", mime_compressed_data,
                   mime_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket, "socket.lua", socket_compressed_data,
                   socket_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_ftp, "socket.ftp.lua",
                   socket_ftp_compressed_data, socket_ftp_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_headers, "socket.headers.lua",
                   socket_headers_compressed_data,
                   socket_headers_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_http, "socket.http.lua",
                   socket_http_compressed_data, socket_http_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_smtp, "socket.smtp.lua",
                   socket_smtp_compressed_data, socket_smtp_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_tp, "socket.tp.lua",
                   socket_tp_compressed_data, socket_tp_compressed_size);

LUAOPEN_EMBED_DATA(open_embed_socket_url, "socket.url.lua",
                   socket_url_compressed_data, socket_url_compressed_size);

void open_luasocket(lua_State *L) {
  package_preload(L, "socket.core", luaopen_socket_core);
  package_preload(L, "mime.core", luaopen_mime_core);

  package_preload(L, "ltn12", open_embed_ltn12);
  package_preload(L, "mbox", open_embed_mbox);
  package_preload(L, "mime", open_embed_mime);
  package_preload(L, "socket", open_embed_socket);
  package_preload(L, "socket.ftp", open_embed_socket_ftp);
  package_preload(L, "socket.headers", open_embed_socket_headers);
  package_preload(L, "socket.http", open_embed_socket_http);
  package_preload(L, "socket.smtp", open_embed_socket_smtp);
  package_preload(L, "socket.tp", open_embed_socket_tp);
  package_preload(L, "socket.url", open_embed_socket_url);
}
#endif // IS_HTML5
