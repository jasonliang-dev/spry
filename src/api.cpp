#include "api.h"
#include "app.h"
#include "atlas.h"
#include "audio.h"
#include "box2d/b2_body.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_contact.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_math.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_world.h"
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
#include "sprite.h"
#include "tilemap.h"

static Color top_color() {
  return g_app->draw_colors[g_app->draw_colors_len - 1];
}

// mt_image

static int mt_image_gc(lua_State *L) {
  Image *img = (Image *)luaL_checkudata(L, 1, "mt_image");
  drop(img);
  return 0;
}

static int mt_image_draw(lua_State *L) {
  Image *img = (Image *)luaL_checkudata(L, 1, "mt_image");
  DrawDescription dd = luax_draw_description(L, 2);
  draw(img, &dd, top_color());
  return 0;
}

static int mt_image_width(lua_State *L) {
  Image *img = (Image *)luaL_checkudata(L, 1, "mt_image");
  lua_pushnumber(L, img->width);
  return 1;
}

static int mt_image_height(lua_State *L) {
  Image *img = (Image *)luaL_checkudata(L, 1, "mt_image");
  lua_pushnumber(L, img->height);
  return 1;
}

static int open_mt_image(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_image_gc},     {"draw", mt_image_draw},
      {"width", mt_image_width}, {"height", mt_image_height},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_image", reg);
  return 1;
}

// mt_font

static int mt_font_gc(lua_State *L) {
  FontFamily **udata = (FontFamily **)luaL_checkudata(L, 1, "mt_font");
  FontFamily *font = *udata;

  if (font != g_app->default_font) {
    drop(font);
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

  draw(font, (u64)size, (float)x, (float)y, text, top_color());
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
  return 1;
}

// mt_audio

static int mt_audio_gc(lua_State *L) {
  i64 *udata = (i64 *)luaL_checkudata(L, 1, "mt_audio");
  if (*udata != -1) {
    drop(&g_app->audio_sources, *udata);
    *udata = -1;
  }
  return 0;
}

static int mt_audio_play(lua_State *L) {
  i64 *udata = (i64 *)luaL_checkudata(L, 1, "mt_audio");
  audio_play(&g_app->audio_sources, *udata);
  return 0;
}

static int mt_audio_play_loop(lua_State *L) {
  i64 *udata = (i64 *)luaL_checkudata(L, 1, "mt_audio");
  audio_play_loop(&g_app->audio_sources, *udata);
  return 0;
}

static int open_mt_audio(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_audio_gc},   {"destroy", mt_audio_gc},
      {"play", mt_audio_play}, {"play_loop", mt_audio_play_loop},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_audio", reg);
  return 1;
}

// mt_sprite_renderer

static int mt_sprite_renderer_play(lua_State *L) {
  SpriteRenderer *sr =
      (SpriteRenderer *)luaL_checkudata(L, 1, "mt_sprite_renderer");
  String tag = luax_check_string(L, 2);

  sprite_renderer_play(sr, tag);
  return 0;
}

static int mt_sprite_renderer_update(lua_State *L) {
  SpriteRenderer *sr =
      (SpriteRenderer *)luaL_checkudata(L, 1, "mt_sprite_renderer");
  lua_Number dt = luaL_checknumber(L, 2);

  sprite_renderer_update(sr, (float)dt);
  return 0;
}

static int mt_sprite_renderer_draw(lua_State *L) {
  SpriteRenderer *sr =
      (SpriteRenderer *)luaL_checkudata(L, 1, "mt_sprite_renderer");
  DrawDescription dd = luax_draw_description(L, 2);

  draw(sr, &dd, top_color());
  return 0;
}

static int mt_sprite_renderer_width(lua_State *L) {
  SpriteRenderer *sr =
      (SpriteRenderer *)luaL_checkudata(L, 1, "mt_sprite_renderer");
  lua_pushnumber(L, (lua_Number)sr->sprite->width);
  return 1;
}

static int mt_sprite_renderer_height(lua_State *L) {
  SpriteRenderer *sr =
      (SpriteRenderer *)luaL_checkudata(L, 1, "mt_sprite_renderer");
  lua_pushnumber(L, (lua_Number)sr->sprite->height);
  return 1;
}

static int mt_sprite_renderer_set_frame(lua_State *L) {
  SpriteRenderer *sr =
      (SpriteRenderer *)luaL_checkudata(L, 1, "mt_sprite_renderer");
  lua_Integer frame = luaL_checknumber(L, 2);

  sprite_renderer_set_frame(sr, (i32)frame);
  return 0;
}

static int mt_sprite_renderer_total_frames(lua_State *L) {
  SpriteRenderer *sr =
      (SpriteRenderer *)luaL_checkudata(L, 1, "mt_sprite_renderer");
  i32 frames = sr->sprite->frames.len;
  lua_pushinteger(L, frames);
  return 1;
}

static int open_mt_sprite_renderer(lua_State *L) {
  luaL_Reg reg[] = {
      {"play", mt_sprite_renderer_play},
      {"update", mt_sprite_renderer_update},
      {"draw", mt_sprite_renderer_draw},
      {"width", mt_sprite_renderer_width},
      {"height", mt_sprite_renderer_height},
      {"set_frame", mt_sprite_renderer_set_frame},
      {"total_frames", mt_sprite_renderer_total_frames},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_sprite_renderer", reg);
  return 1;
}

// mt_atlas_image

static int mt_atlas_image_draw(lua_State *L) {
  AtlasImage *atlas_img = (AtlasImage *)luaL_checkudata(L, 1, "mt_atlas_image");
  DrawDescription dd = luax_draw_description(L, 2);

  dd.u0 = atlas_img->u0;
  dd.v0 = atlas_img->v0;
  dd.u1 = atlas_img->u1;
  dd.v1 = atlas_img->v1;

  draw(&atlas_img->img, &dd, top_color());
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
  return 1;
}

// mt_atlas

static int mt_atlas_gc(lua_State *L) {
  Atlas *atlas = (Atlas *)luaL_checkudata(L, 1, "mt_atlas");
  drop(atlas);
  return 0;
}

static int mt_atlas_get_image(lua_State *L) {
  Atlas *atlas = (Atlas *)luaL_checkudata(L, 1, "mt_atlas");
  String name = luax_check_string(L, 2);

  AtlasImage *atlas_img = atlas_get(atlas, name);
  if (atlas_img == nullptr) {
    return 0;
  }

  luax_newuserdata(L, *atlas_img, "mt_atlas_image");
  return 1;
}

static int open_mt_atlas(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_atlas_gc},
      {"get_image", mt_atlas_get_image},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_atlas", reg);
  return 1;
}

// mt_tilemap

static int mt_tilemap_gc(lua_State *L) {
  Tilemap *tm = (Tilemap *)luaL_checkudata(L, 1, "mt_tilemap");
  drop(tm);
  return 0;
}

static int mt_tilemap_draw(lua_State *L) {
  Tilemap *tm = (Tilemap *)luaL_checkudata(L, 1, "mt_tilemap");
  draw(tm, top_color());
  return 0;
}

static int mt_tilemap_grid_begin(lua_State *L) {
  Tilemap *tm = (Tilemap *)luaL_checkudata(L, 1, "mt_tilemap");
  String str = luax_check_string(L, 2);
  tilemap_grid_begin(tm, str);
  return 0;
}

static int mt_tilemap_grid_end(lua_State *L) {
  Tilemap *tm = (Tilemap *)luaL_checkudata(L, 1, "mt_tilemap");
  tilemap_grid_end(tm);
  return 0;
}

static int mt_tilemap_grid_value(lua_State *L) {
  Tilemap *tm = (Tilemap *)luaL_checkudata(L, 1, "mt_tilemap");
  lua_Number x = luaL_checknumber(L, 2);
  lua_Number y = luaL_checknumber(L, 3);

  TilemapInt res = tilemap_grid_value(tm, (float)x, (float)y);
  lua_pushinteger(L, res);
  return 1;
}

static int mt_tilemap_grid_rect(lua_State *L) {
  Tilemap *tm = (Tilemap *)luaL_checkudata(L, 1, "mt_tilemap");

  lua_Number x0 = luaL_checknumber(L, 2);
  lua_Number y0 = luaL_checknumber(L, 3);
  lua_Number x1 = luaL_checknumber(L, 4);
  lua_Number y1 = luaL_checknumber(L, 5);

  TilemapInt top_left = tilemap_grid_value(tm, (float)x0, (float)y0);
  TilemapInt top_right = tilemap_grid_value(tm, (float)x1, (float)y0);
  TilemapInt bot_left = tilemap_grid_value(tm, (float)x0, (float)y1);
  TilemapInt bot_right = tilemap_grid_value(tm, (float)x1, (float)y1);

  lua_pushinteger(L, top_left);
  lua_pushinteger(L, top_right);
  lua_pushinteger(L, bot_left);
  lua_pushinteger(L, bot_right);
  return 4;
}

static int mt_tilemap_rect_every(lua_State *L) {
  Tilemap *tm = (Tilemap *)luaL_checkudata(L, 1, "mt_tilemap");

  lua_Integer needle = luaL_checkinteger(L, 2);

  float x0 = (float)luaL_checknumber(L, 3);
  float y0 = (float)luaL_checknumber(L, 4);
  float x1 = (float)luaL_checknumber(L, 5);
  float y1 = (float)luaL_checknumber(L, 6);

  bool every_tile = tilemap_rect_every(tm, (TilemapInt)needle, x0, y0, x1, y1);
  lua_pushboolean(L, every_tile);
  return 1;
}

static int mt_tilemap_rect_has(lua_State *L) {
  Tilemap *tm = (Tilemap *)luaL_checkudata(L, 1, "mt_tilemap");

  lua_Integer needle = luaL_checkinteger(L, 2);

  float x0 = (float)luaL_checknumber(L, 3);
  float y0 = (float)luaL_checknumber(L, 4);
  float x1 = (float)luaL_checknumber(L, 5);
  float y1 = (float)luaL_checknumber(L, 6);

  bool every_tile = tilemap_rect_has(tm, (TilemapInt)needle, x0, y0, x1, y1);
  lua_pushboolean(L, every_tile);
  return 1;
}

static int mt_tilemap_entities(lua_State *L) {
  Tilemap *tm = (Tilemap *)luaL_checkudata(L, 1, "mt_tilemap");

  u64 entities = 0;
  for (TilemapLevel &level : tm->levels) {
    for (TilemapLayer &layer : level.layers) {
      entities += layer.entities.len;
    }
  }

  lua_createtable(L, (i32)entities, 0);

  i32 i = 1;
  for (TilemapLevel &level : tm->levels) {
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

static int open_mt_tilemap(lua_State *L) {
  luaL_Reg reg[] = {
      {"__gc", mt_tilemap_gc},
      {"draw", mt_tilemap_draw},
      {"grid_begin", mt_tilemap_grid_begin},
      {"grid_end", mt_tilemap_grid_end},
      {"grid_value", mt_tilemap_grid_value},
      {"grid_rect", mt_tilemap_grid_rect},
      {"rect_every", mt_tilemap_rect_every},
      {"rect_has", mt_tilemap_rect_has},
      {"entities", mt_tilemap_entities},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_tilemap", reg);
  return 1;
}

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

static void drop(lua_State *L, PhysicsUserData *pud) {
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
  assert(pud != nullptr);

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

    luax_newuserdata(L, a, "mt_b2_fixture");
    luax_newuserdata(L, b, "mt_b2_fixture");

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

  luax_newuserdata(L, p, "mt_b2_body");
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
  return 1;
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
      defer(drop(&puds));

      for (b2Fixture *f = physics->body->GetFixtureList(); f != nullptr;
           f = f->GetNext()) {
        PhysicsUserData *p = (PhysicsUserData *)f->GetUserData().pointer;
        push(&puds, p);
      }
      push(&puds, pud);

      physics->world->DestroyBody(physics->body);
      physics->body = nullptr;

      for (PhysicsUserData *pud : puds) {
        drop(L, pud);
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

  b2FixtureDef fixture_def;
  fixture_def.isSensor = sensor;
  fixture_def.density = (float)density;
  fixture_def.friction = (float)friction;
  fixture_def.restitution = (float)restitution;
  fixture_def.userData.pointer = (u64)pud;
  return fixture_def;
}

static int mt_b2_body_make_box_fixture(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;
  b2FixtureDef fixture_def = b2_fixture_def(L);

  lua_Number w = luax_number_field(L, "w");
  lua_Number h = luax_number_field(L, "h");

  b2PolygonShape box;
  box.SetAsBox((float)w / physics->meter, (float)h / physics->meter);
  fixture_def.shape = &box;

  Physics p = weak_copy(physics);
  p.fixture = body->CreateFixture(&fixture_def);

  luax_newuserdata(L, p, "mt_b2_fixture");
  return 0;
}

static int mt_b2_body_make_circle_fixture(lua_State *L) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_body");
  b2Body *body = physics->body;
  b2FixtureDef fixture_def = b2_fixture_def(L);

  lua_Number radius = luax_number_field(L, "radius");

  b2CircleShape circle;
  circle.m_radius = radius / physics->meter;
  fixture_def.shape = &circle;

  Physics p = weak_copy(physics);
  p.fixture = body->CreateFixture(&fixture_def);

  luax_newuserdata(L, p, "mt_b2_fixture");
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

  for (b2Fixture *f = body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
    switch (f->GetType()) {
    case b2Shape::e_circle: {
      b2CircleShape *circle = (b2CircleShape *)f->GetShape();
      b2Vec2 pos = body->GetWorldPoint(circle->m_p);
      draw_line_circle(pos.x * physics->meter, pos.y * physics->meter,
                       circle->m_radius * physics->meter, top_color());
      break;
    }
    case b2Shape::e_polygon: {
      b2PolygonShape *poly = (b2PolygonShape *)f->GetShape();

      if (poly->m_count > 0) {
        sgl_disable_texture();
        sgl_begin_line_strip();

        Color c = top_color();
        sgl_c4b(c.r, c.g, c.b, c.a);

        for (i32 i = 0; i < poly->m_count; i++) {
          b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[i]);
          sgl_v2f(pos.x * physics->meter, pos.y * physics->meter);
        }

        b2Vec2 pos = body->GetWorldPoint(poly->m_vertices[0]);
        sgl_v2f(pos.x * physics->meter, pos.y * physics->meter);

        sgl_end();
      }
      break;
    }
    default: break;
    }
  }

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
      {"fixed_rotation", mt_b2_body_fixed_rotation},
      {"apply_force", mt_b2_body_apply_force},
      {"apply_impulse", mt_b2_body_apply_impulse},
      {"set_position", mt_b2_body_set_position},
      {"set_velocity", mt_b2_body_set_velocity},
      {"set_angle", mt_b2_body_set_angle},
      {"set_fixed_rotation", mt_b2_body_set_fixed_rotation},
      {"set_transform", mt_b2_body_set_transform},
      {"draw_fixtures", mt_b2_body_draw_fixtures},
      {"udata", mt_b2_body_udata},
      {nullptr, nullptr},
  };

  luax_new_class(L, "mt_b2_body", reg);
  return 1;
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
  lua_Number dt = luaL_optnumber(L, 2, g_app->delta_time);
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
  bool fixed_rotation = luax_boolean_field(L, "fixed_rotation");
  PhysicsUserData *pud = physics_userdata(L);

  b2BodyDef body_def = {};
  body_def.position.Set((float)x / physics->meter, (float)y / physics->meter);
  body_def.linearVelocity.Set((float)vx / physics->meter,
                              (float)vy / physics->meter);
  body_def.angle = angle;
  body_def.fixedRotation = fixed_rotation;
  body_def.userData.pointer = (u64)pud;
  return body_def;
}

static int b2_make_body(lua_State *L, b2BodyType type) {
  Physics *physics = (Physics *)luaL_checkudata(L, 1, "mt_b2_world");
  b2BodyDef body_def = b2_body_def(L, physics);
  body_def.type = type;

  Physics p = weak_copy(physics);
  p.body = physics->world->CreateBody(&body_def);

  luax_newuserdata(L, p, "mt_b2_body");
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
  return 1;
}

// spry api

static int quit(lua_State *L) {
  (void)L;
  sapp_request_quit();
  return 0;
}

static int platform(lua_State *L) {
#if defined(__EMSCRIPTEN__)
  lua_pushliteral(L, "html5");
#elif defined(_WIN32)
  lua_pushliteral(L, "windows");
#elif defined(__linux__) || defined(__unix__)
  lua_pushliteral(L, "linux");
#endif
  return 1;
}

static int dt(lua_State *L) {
  lua_pushnumber(L, g_app->delta_time);
  return 1;
}

static int window_width(lua_State *L) {
  float width = sapp_widthf();
  lua_pushnumber(L, width);
  return 1;
}

static int window_height(lua_State *L) {
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

static int key_down(lua_State *L) {
  String str = luax_check_string(L, 1);
  i32 key = keyboard_lookup(str);
  bool is_down = g_app->key_state[key];
  lua_pushboolean(L, is_down);
  return 1;
}

static int key_release(lua_State *L) {
  String str = luax_check_string(L, 1);
  i32 key = keyboard_lookup(str);
  bool is_release = !g_app->key_state[key] && g_app->prev_key_state[key];
  lua_pushboolean(L, is_release);
  return 1;
}

static int key_press(lua_State *L) {
  String str = luax_check_string(L, 1);
  i32 key = keyboard_lookup(str);
  bool is_press = g_app->key_state[key] && !g_app->prev_key_state[key];
  lua_pushboolean(L, is_press);
  return 1;
}

static int mouse_down(lua_State *L) {
  lua_Integer n = luaL_checkinteger(L, 1);
  if (n >= 0 && n < array_size(g_app->mouse_state)) {
    lua_pushboolean(L, g_app->mouse_state[n]);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}

static int mouse_release(lua_State *L) {
  lua_Integer n = luaL_checkinteger(L, 1);
  if (n >= 0 && n < array_size(g_app->mouse_state)) {
    bool is_release = !g_app->mouse_state[n] && g_app->prev_mouse_state[n];
    lua_pushboolean(L, is_release);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}

static int mouse_click(lua_State *L) {
  lua_Integer n = luaL_checkinteger(L, 1);
  if (n >= 0 && n < array_size(g_app->mouse_state)) {
    bool is_click = g_app->mouse_state[n] && !g_app->prev_mouse_state[n];
    lua_pushboolean(L, is_click);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}

static int mouse_pos(lua_State *L) {
  lua_pushnumber(L, g_app->mouse_x);
  lua_pushnumber(L, g_app->mouse_y);
  return 2;
}

static int mouse_delta(lua_State *L) {
  lua_pushnumber(L, g_app->mouse_dx);
  lua_pushnumber(L, g_app->mouse_dy);
  return 2;
}

static int show_mouse(lua_State *L) {
  bool show = lua_toboolean(L, 1);
  sapp_show_mouse(show);
  return 0;
}

static int scroll_wheel(lua_State *L) {
  lua_pushnumber(L, g_app->scroll_x);
  lua_pushnumber(L, g_app->scroll_y);
  return 2;
}

static int push_matrix(lua_State *L) {
  sgl_push_matrix();
  return 0;
}

static int pop_matrix(lua_State *L) {
  sgl_pop_matrix();
  return 0;
}

static int translate(lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);

  sgl_translate((float)x, (float)y, 0);
  return 0;
}

static int rotate(lua_State *L) {
  lua_Number angle = luaL_checknumber(L, 1);

  sgl_rotate((float)angle, 0, 0, 1);
  return 0;
}

static int scale(lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);

  sgl_scale((float)x, (float)y, 1);
  return 0;
}

static int clear_color(lua_State *L) {
  lua_Number r = luaL_checknumber(L, 1);
  lua_Number g = luaL_checknumber(L, 2);
  lua_Number b = luaL_checknumber(L, 3);
  lua_Number a = luaL_checknumber(L, 4);

  g_app->clear_color[0] = (float)r / 255.0f;
  g_app->clear_color[1] = (float)g / 255.0f;
  g_app->clear_color[2] = (float)b / 255.0f;
  g_app->clear_color[3] = (float)a / 255.0f;

  return 0;
}

static int push_color(lua_State *L) {
  lua_Number r = luaL_checknumber(L, 1);
  lua_Number g = luaL_checknumber(L, 2);
  lua_Number b = luaL_checknumber(L, 3);
  lua_Number a = luaL_checknumber(L, 4);

  Color color = {};
  color.r = (u8)r;
  color.g = (u8)g;
  color.b = (u8)b;
  color.a = (u8)a;

  if (g_app->draw_colors_len == array_size(g_app->draw_colors)) {
    return luaL_error(L, "color stack is full");
  }

  g_app->draw_colors[g_app->draw_colors_len++] = color;
  return 0;
}

static int pop_color(lua_State *L) {
  if (g_app->draw_colors_len == 1) {
    return luaL_error(L, "color stack can't be less than 1");
  }

  g_app->draw_colors_len--;
  return 0;
}

static int default_font(lua_State *L) {
  if (!g_app->default_font_loaded) {
    g_app->default_font = (FontFamily *)mem_alloc(sizeof(FontFamily));
    font_load_default(g_app->default_font);
    g_app->default_font_loaded = true;
  }

  // NOLINTNEXTLINE(bugprone-sizeof-expression)
  luax_newuserdata(L, g_app->default_font, "mt_font");
  return 1;
}

static int draw_filled_rect(lua_State *L) {
  RectDescription rd = luax_rect_description(L, 1);
  draw_filled_rect(&rd, top_color());
  return 0;
}

static int draw_line_rect(lua_State *L) {
  RectDescription rd = luax_rect_description(L, 1);
  draw_line_rect(&rd, top_color());
  return 0;
}

static int draw_line_circle(lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);
  lua_Number radius = luaL_checknumber(L, 3);

  draw_line_circle(x, y, radius, top_color());
  return 0;
}

static int image_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  Image img;
  bool ok = image_load(&img, &g_app->archive, str);
  if (!ok) {
    return 0;
  }

  luax_newuserdata(L, img, "mt_image");
  return 1;
}

static int font_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  FontFamily *font = (FontFamily *)mem_alloc(sizeof(FontFamily));
  bool ok = font_load(font, &g_app->archive, str);
  if (!ok) {
    return 0;
  }

  // NOLINTNEXTLINE(bugprone-sizeof-expression)
  luax_newuserdata(L, font, "mt_font");
  return 1;
}

static int audio_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  i64 index = audio_load(&g_app->audio_sources, &g_app->archive, str);
  if (index == -1) {
    return 0;
  }

  luax_newuserdata(L, index, "mt_audio");
  return 1;
}

static int sprite_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  u64 key = fnv1a(str);
  Sprite *spr = nullptr;
  bool ok = get(&g_app->sprites, key, &spr);
  if (!ok) {
    bool ok = sprite_load(spr, &g_app->archive, str);
    if (!ok) {
      return 0;
    }
  }

  SpriteRenderer sr = {};
  sr.sprite = spr;

  luax_newuserdata(L, sr, "mt_sprite_renderer");
  return 1;
}

static int atlas_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  Atlas atlas;
  bool ok = atlas_load(&atlas, &g_app->archive, str);
  if (!ok) {
    return 0;
  }

  luax_newuserdata(L, atlas, "mt_atlas");
  return 1;
}

static int tilemap_load(lua_State *L) {
  String str = luax_check_string(L, 1);

  Tilemap tm;
  bool ok = tilemap_load(&tm, &g_app->archive, str);
  if (!ok) {
    return 0;
  }

  luax_newuserdata(L, tm, "mt_tilemap");
  return 1;
}

static int b2_world(lua_State *L) {
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

  luax_newuserdata(L, physics, "mt_b2_world");
  return 1;
}

static int open_spry(lua_State *L) {
  luaL_Reg reg[] = {
      {"quit", quit},
      {"platform", platform},
      {"dt", dt},
      {"window_width", window_width},
      {"window_height", window_height},
      {"key_down", key_down},
      {"key_release", key_release},
      {"key_press", key_press},
      {"mouse_down", mouse_down},
      {"mouse_release", mouse_release},
      {"mouse_click", mouse_click},
      {"mouse_pos", mouse_pos},
      {"mouse_delta", mouse_delta},
      {"show_mouse", show_mouse},
      {"scroll_wheel", scroll_wheel},
      {"push_matrix", push_matrix},
      {"pop_matrix", pop_matrix},
      {"translate", translate},
      {"rotate", rotate},
      {"scale", scale},
      {"clear_color", clear_color},
      {"push_color", push_color},
      {"pop_color", pop_color},
      {"default_font", default_font},
      {"draw_filled_rect", draw_filled_rect},
      {"draw_line_rect", draw_line_rect},
      {"draw_line_circle", draw_line_circle},
      {"image_load", image_load},
      {"font_load", font_load},
      {"audio_load", audio_load},
      {"sprite_load", sprite_load},
      {"atlas_load", atlas_load},
      {"tilemap_load", tilemap_load},
      {"b2_world", b2_world},
      {nullptr, nullptr},
  };

  luaL_newlib(L, reg);
  return 1;
}

void open_spry_api(lua_State *L) {
  lua_CFunction mt_funcs[] = {
      open_mt_image,           open_mt_font,        open_mt_audio,
      open_mt_sprite_renderer, open_mt_atlas_image, open_mt_atlas,
      open_mt_tilemap,         open_mt_b2_fixture,  open_mt_b2_body,
      open_mt_b2_world,
  };

  for (u32 i = 0; i < array_size(mt_funcs); i++) {
    mt_funcs[i](L);
    lua_pop(L, 1);
  }

  luaL_requiref(L, "spry", open_spry, 1);
  lua_pop(L, 1);
}
