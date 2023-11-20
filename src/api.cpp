#include "api.h"
#include "app.h"
#include "assets.h"
#include "atlas.h"
#include "deps/lua/lauxlib.h"
#include "deps/lua/lua.h"
#include "deps/sokol_app.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "draw.h"
#include "font.h"
#include "image.h"
#include "luax.h"
#include "prelude.h"
#include "profile.h"
#include "sound.h"
#include "sprite.h"
#include "tilemap.h"
#include <box2d/b2_body.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_math.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>

struct PhysicsContactListener;
struct Physics {
  b2World *world;
  PhysicsContactListener *contact_listener;
  float meter;

  union {
    b2Body *body;
    b2Fixture *fixture;
  };
};

struct PhysicsUserData {
  i32 begin_contact_ref;
  i32 end_contact_ref;

  i32 ref_count;
  i32 type;
  union {
    char *str;
    lua_Number num;
  };
};

static void drop_physics_udata(lua_State *L, PhysicsUserData *pud) {
  if (pud->type == LUA_TSTRING) {
    mem_free(pud->str);
  }

  if (pud->begin_contact_ref != LUA_REFNIL) {
    assert(pud->begin_contact_ref != 0);
    luaL_unref(L, LUA_REGISTRYINDEX, pud->begin_contact_ref);
  }

  if (pud->end_contact_ref != LUA_REFNIL) {
    assert(pud->end_contact_ref != 0);
    luaL_unref(L, LUA_REGISTRYINDEX, pud->end_contact_ref);
  }
}

static PhysicsUserData *physics_userdata(lua_State *L) {
  PhysicsUserData *pud = (PhysicsUserData *)mem_alloc(sizeof(PhysicsUserData));

  pud->type = lua_getfield(L, -1, "udata");
  switch (pud->type) {
  case LUA_TNUMBER: pud->num = luaL_checknumber(L, -1); break;
  case LUA_TSTRING: pud->str = to_cstr(luaL_checkstring(L, -1)).data; break;
  default: break;
  }
  lua_pop(L, 1);

  lua_getfield(L, -1, "begin_contact");
  pud->begin_contact_ref = luaL_ref(L, LUA_REGISTRYINDEX);

  lua_getfield(L, -1, "end_contact");
  pud->end_contact_ref = luaL_ref(L, LUA_REGISTRYINDEX);

  pud->ref_count = 1;
  return pud;
}

static void physics_push_userdata(lua_State *L, u64 ptr) {
  PhysicsUserData *pud = (PhysicsUserData *)ptr;

  if (pud == nullptr) {
    lua_pushnil(L);
    return;
  }

  switch (pud->type) {
  case LUA_TNUMBER: lua_pushnumber(L, pud->num); break;
  case LUA_TSTRING: lua_pushstring(L, pud->str); break;
  default: lua_pushnil(L); break;
  }
}

Physics weak_copy(Physics *p) {
  Physics physics = {};
  physics.world = p->world;
  physics.contact_listener = p->contact_listener;
  physics.meter = p->meter;
  return physics;
}

static void contact_run_cb(lua_State *L, i32 ref, i32 a, i32 b, i32 msgh) {
  if (ref != LUA_REFNIL) {
    assert(ref != 0);
    i32 type = lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    if (type != LUA_TFUNCTION) {
      luaL_error(L, "expected contact listener to be a callback");
      return;
    }
    i32 top = lua_gettop(L);
    lua_pushvalue(L, top + a);
    lua_pushvalue(L, top + b);
    lua_pcall(L, 2, 0, msgh);
  }
}

struct PhysicsContactListener : public b2ContactListener {
  lua_State *L = nullptr;
  Physics physics = {};
  i32 begin_contact_ref = LUA_REFNIL;
  i32 end_contact_ref = LUA_REFNIL;

  void setup_contact(b2Contact *contact, i32 *msgh, PhysicsUserData **pud_a,
                     PhysicsUserData **pud_b) {
    lua_pushcfunction(L, luax_msgh);
    *msgh = lua_gettop(L);

    Physics a = weak_copy(&physics);
    a.fixture = contact->GetFixtureA();

    Physics b = weak_copy(&physics);
    b.fixture = contact->GetFixtureB();

    luax_new_userdata(L, a, "mt_b2_fixture");
    luax_new_userdata(L, b, "mt_b2_fixture");

    *pud_a = (PhysicsUserData *)a.fixture->GetUserData().pointer;
    *pud_b = (PhysicsUserData *)b.fixture->GetUserData().pointer;
  }

  void BeginContact(b2Contact *contact) {
    i32 msgh = 0;
    PhysicsUserData *pud_a = nullptr;
    PhysicsUserData *pud_b = nullptr;
    setup_contact(contact, &msgh, &pud_a, &pud_b);

    contact_run_cb(L, begin_contact_ref, -2, -1, msgh);
    if (pud_a) {
      contact_run_cb(L, pud_a->begin_contact_ref, -2, -1, msgh);
    }
    if (pud_b) {
      contact_run_cb(L, pud_b->begin_contact_ref, -1, -2, msgh);
    }

    lua_pop(L, 2);
  }

  void EndContact(b2Contact *contact) {
    i32 msgh = 0;
    PhysicsUserData *pud_a = nullptr;
    PhysicsUserData *pud_b = nullptr;
    setup_contact(contact, &msgh, &pud_a, &pud_b);

    contact_run_cb(L, end_contact_ref, -2, -1, msgh);
    if (pud_a) {
      contact_run_cb(L, pud_a->end_contact_ref, -2, -1, msgh);
    }
    if (pud_b) {
      contact_run_cb(L, pud_b->end_contact_ref, -1, -2, msgh);
    }

    lua_pop(L, 2);
  }
};

static void draw_fixtures_for_body(b2Body *body, float meter) {
  for (b2Fixture *f = body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
    switch (f->GetType()) {
    case b2Shape::e_circle: {
      b2CircleShape *circle = (b2CircleShape *)f->GetShape();
      b2Vec2 pos = body->GetWorldPoint(circle->m_p);
      draw_line_circle(pos.x * meter, pos.y * meter, circle->m_radius * meter);
      break;
    }
    case b2Shape::e_polygon: {
      b2PolygonShape *poly = (b2PolygonShape *)f->GetShape();

      if (poly->m_count > 0) {
        sgl_disable_texture();
        sgl_begin_line_strip();

        renderer_apply_color();

        for (i32 i = 0; i < poly->m_count; i++) {
          b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[i]);
          renderer_push_xy(pos.x * meter, pos.y * meter);
        }

        b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[0]);
        renderer_push_xy(pos.x * meter, pos.y * meter);

        sgl_end();
      }
      break;
    }
    default: break;
    }
  }
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

  draw_font(font, (u64)size, (float)x, (float)y, text);
  return 0;
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

        luax_set_field(L, "id", entity.identifier.data);
        luax_set_field(L, "x", entity.x + level.world_x);
        luax_set_field(L, "y", entity.y + level.world_y);

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

  lua_Unsigned n = lua_rawlen(L, 4);
  for (i32 i = 1; i <= n; i++) {
    lua_rawgeti(L, 4, i);
    lua_Number tile = luaL_checknumber(L, -1);
    array_push(&walls, (TilemapInt)tile);
    lua_pop(L, 1);
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
  lua_pushnil(L);

  while (lua_next(L, -2)) {
    lua_Number value = luaL_checknumber(L, -1);
    lua_Number key = luaL_checknumber(L, -2);

    TileCost cost = {};
    cost.cell = (TilemapInt)key;
    cost.value = (float)value;

    array_push(&costs, cost);
    lua_pop(L, 1);
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

      luax_set_field(L, "x", n->x * asset.tilemap.graph_grid_size);
      luax_set_field(L, "y", n->y * asset.tilemap.graph_grid_size);

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

  Physics p = weak_copy(physics);
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
      Array<PhysicsUserData *> puds = {};
      defer(array_trash(&puds));

      for (b2Fixture *f = physics->body->GetFixtureList(); f != nullptr;
           f = f->GetNext()) {
        PhysicsUserData *p = (PhysicsUserData *)f->GetUserData().pointer;
        array_push(&puds, p);
      }
      array_push(&puds, pud);

      physics->world->DestroyBody(physics->body);
      physics->body = nullptr;

      for (PhysicsUserData *pud : puds) {
        drop_physics_udata(L, pud);
        mem_free(pud);
      }
    }
  }

  return 0;
}

static int mt_b2_body_gc(lua_State *L) { return b2_body_unref(L, false); }

static int mt_b2_body_destroy(lua_State *L) { return b2_body_unref(L, true); }

static b2FixtureDef b2_fixture_def(lua_State *L) {
  bool sensor = luax_boolean_field(L, "sensor");
  lua_Number density = luax_number_field(L, "density", 1);
  lua_Number friction = luax_number_field(L, "friction", 0.2);
  lua_Number restitution = luax_number_field(L, "restitution", 0);
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
  b2FixtureDef fixture_def = b2_fixture_def(L);

  lua_Number x = luax_number_field(L, "x", 0);
  lua_Number y = luax_number_field(L, "y", 0);
  lua_Number w = luax_number_field(L, "w");
  lua_Number h = luax_number_field(L, "h");
  lua_Number angle = luax_number_field(L, "angle", 0);

  b2Vec2 pos = {(float)x / physics->meter, (float)y / physics->meter};

  b2PolygonShape box = {};
  box.SetAsBox((float)w / physics->meter, (float)h / physics->meter, pos,
               angle);
  fixture_def.shape = &box;

  Physics p = weak_copy(physics);
  p.fixture = body->CreateFixture(&fixture_def);

  luax_new_userdata(L, p, "mt_b2_fixture");
  return 0;
}

static int mt_b2_body_make_circle_fixture(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;
  b2FixtureDef fixture_def = b2_fixture_def(L);

  lua_Number x = luax_number_field(L, "x", 0);
  lua_Number y = luax_number_field(L, "y", 0);
  lua_Number radius = luax_number_field(L, "radius");

  b2CircleShape circle = {};
  circle.m_radius = radius / physics->meter;
  circle.m_p = {(float)x / physics->meter, (float)y / physics->meter};
  fixture_def.shape = &circle;

  Physics p = weak_copy(physics);
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

  if (physics->world != nullptr) {
    if (physics->contact_listener->begin_contact_ref != LUA_REFNIL) {
      luaL_unref(L, LUA_REGISTRYINDEX,
                 physics->contact_listener->begin_contact_ref);
    }
    if (physics->contact_listener->end_contact_ref != LUA_REFNIL) {
      luaL_unref(L, LUA_REGISTRYINDEX,
                 physics->contact_listener->end_contact_ref);
    }

    delete physics->contact_listener;
    delete physics->world;
    physics->contact_listener = nullptr;
    physics->world = nullptr;
  }

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

static b2BodyDef b2_body_def(lua_State *L, Physics *physics) {
  lua_Number x = luax_number_field(L, "x");
  lua_Number y = luax_number_field(L, "y");
  lua_Number vx = luax_number_field(L, "vx", 0);
  lua_Number vy = luax_number_field(L, "vy", 0);
  lua_Number angle = luax_number_field(L, "angle", 0);
  lua_Number linear_damping = luax_number_field(L, "linear_damping", 0);
  bool fixed_rotation = luax_boolean_field(L, "fixed_rotation");
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
  b2BodyDef body_def = b2_body_def(L, physics);
  body_def.type = type;

  Physics p = weak_copy(physics);
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

  if (physics->contact_listener->begin_contact_ref != LUA_REFNIL) {
    luaL_unref(L, LUA_REGISTRYINDEX,
               physics->contact_listener->begin_contact_ref);
  }

  lua_pushvalue(L, 2);
  i32 ref = luaL_ref(L, LUA_REGISTRYINDEX);
  physics->contact_listener->begin_contact_ref = ref;
  return 0;
}

static int mt_b2_world_end_contact(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
  if (lua_type(L, 2) != LUA_TFUNCTION) {
    return luaL_error(L, "expected argument 2 to be a function");
  }

  if (physics->contact_listener->end_contact_ref != LUA_REFNIL) {
    luaL_unref(L, LUA_REGISTRYINDEX,
               physics->contact_listener->end_contact_ref);
  }

  lua_pushvalue(L, 2);
  i32 ref = luaL_ref(L, LUA_REGISTRYINDEX);
  physics->contact_listener->end_contact_ref = ref;
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

// spry api

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
  String min_filter = luax_string_field(L, "min_filter", "nearest");
  String mag_filter = luax_string_field(L, "mag_filter", "nearest");
  String wrap_u = luax_string_field(L, "wrap_u", "repeat");
  String wrap_v = luax_string_field(L, "wrap_v", "repeat");

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
  lua_Number gx = luax_number_field(L, "gx", 0);
  lua_Number gy = luax_number_field(L, "gy", 9.81);
  lua_Number meter = luax_number_field(L, "meter", 16);

  b2Vec2 gravity = {(float)gx, (float)gy};

  Physics physics = {};
  physics.world = new b2World(gravity);
  physics.meter = meter;
  physics.contact_listener = new PhysicsContactListener;
  physics.contact_listener->L = L;
  physics.contact_listener->physics = weak_copy(&physics);

  physics.world->SetContactListener(physics.contact_listener);

  luax_new_userdata(L, physics, "mt_b2_world");
  return 1;
}

static int open_spry(lua_State *L) {
  luaL_Reg reg[] = {
      // internal
      {"_require_lua_script", spry_require_lua_script},

      // core
      {"version", spry_version},
      {"quit", spry_quit},
      {"platform", spry_platform},
      {"dt", spry_dt},
      {"fullscreen", spry_fullscreen},
      {"toggle_fullscreen", spry_toggle_fullscreen},
      {"window_width", spry_window_width},
      {"window_height", spry_window_height},

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
      open_mt_sampler, open_mt_image,    open_mt_font,
      open_mt_sound,   open_mt_sprite,   open_mt_atlas_image,
      open_mt_atlas,   open_mt_tilemap,  open_mt_b2_fixture,
      open_mt_b2_body, open_mt_b2_world,
  };

  for (u32 i = 0; i < array_size(mt_funcs); i++) {
    mt_funcs[i](L);
  }

  luaL_requiref(L, "spry", open_spry, 1);
  lua_pop(L, 1);
}
