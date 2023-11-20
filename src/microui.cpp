#include "microui.h"
#include "app.h"
#include "deps/microui_atlas.inl"
#include "deps/sokol_app.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "luax.h"
#include "prelude.h"

struct MicrouiState {
  mu_Context *ctx;
  u32 atlas;
};

static MicrouiState g_mui_state;

mu_Context *microui_ctx() { return g_mui_state.ctx; }

void microui_init() {
  mu_Context *ctx = (mu_Context *)mem_alloc(sizeof(mu_Context));
  mu_init(ctx);

  ctx->text_width = [](mu_Font font, const char *text, int len) -> int {
    if (len == -1) {
      len = strlen(text);
    }

    int res = 0;
    for (const char *p = text; *p && len--; p++) {
      res += mu_atlas_lookup(MU_ATLAS_FONT + (unsigned char)*p).w;
    }
    return res;
  };

  ctx->text_height = [](mu_Font font) -> int { return 18; };

  g_mui_state.ctx = ctx;

  u32 *bitmap = (u32 *)mem_alloc(MU_ATLAS_WIDTH * MU_ATLAS_HEIGHT * 4);
  defer(mem_free(bitmap));

  for (i32 i = 0; i < MU_ATLAS_WIDTH * MU_ATLAS_HEIGHT; i++) {
    bitmap[i] = 0x00FFFFFF | ((u32)mu_atlas_texture[i] << 24);
  }

  sg_image_desc desc = {};
  desc.width = MU_ATLAS_WIDTH;
  desc.height = MU_ATLAS_HEIGHT;
  desc.data.subimage[0][0].ptr = bitmap;
  desc.data.subimage[0][0].size = MU_ATLAS_WIDTH * MU_ATLAS_HEIGHT * 4;
  g_mui_state.atlas = sg_make_image(&desc).id;
}

void microui_trash() { mem_free(g_mui_state.ctx); }

static char mui_key_map(sapp_keycode code) {
  switch (code & 511) {
  case SAPP_KEYCODE_LEFT_SHIFT: return MU_KEY_SHIFT;
  case SAPP_KEYCODE_RIGHT_SHIFT: return MU_KEY_SHIFT;
  case SAPP_KEYCODE_LEFT_CONTROL: return MU_KEY_CTRL;
  case SAPP_KEYCODE_RIGHT_CONTROL: return MU_KEY_CTRL;
  case SAPP_KEYCODE_LEFT_ALT: return MU_KEY_ALT;
  case SAPP_KEYCODE_RIGHT_ALT: return MU_KEY_ALT;
  case SAPP_KEYCODE_ENTER: return MU_KEY_RETURN;
  case SAPP_KEYCODE_BACKSPACE: return MU_KEY_BACKSPACE;
  default: return 0;
  }
}

void microui_sokol_event(const sapp_event *e) {
  switch (e->type) {
  case SAPP_EVENTTYPE_CHAR: {
    char str[2] = {(char)(e->char_code % 256), 0};
    mu_input_text(g_mui_state.ctx, str);
    break;
  }
  case SAPP_EVENTTYPE_KEY_DOWN:
    mu_input_keydown(g_mui_state.ctx, mui_key_map(e->key_code));
    break;
  case SAPP_EVENTTYPE_KEY_UP:
    mu_input_keyup(g_mui_state.ctx, mui_key_map(e->key_code));
    break;
  case SAPP_EVENTTYPE_MOUSE_DOWN:
    mu_input_mousedown(g_mui_state.ctx, e->mouse_x, e->mouse_y,
                       (1 << e->mouse_button));
    break;
  case SAPP_EVENTTYPE_MOUSE_UP:
    mu_input_mouseup(g_mui_state.ctx, e->mouse_x, e->mouse_y,
                     (1 << e->mouse_button));
    break;
  case SAPP_EVENTTYPE_MOUSE_MOVE:
    mu_input_mousemove(g_mui_state.ctx, e->mouse_x, e->mouse_y);
    break;
  case SAPP_EVENTTYPE_MOUSE_SCROLL:
    mu_input_scroll(g_mui_state.ctx, e->scroll_x * 5, -e->scroll_y * 5);
    break;
  default: break;
  }
}

static void mu_push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
  sgl_begin_quads();

  float u0 = (float)src.x / (float)MU_ATLAS_WIDTH;
  float v0 = (float)src.y / (float)MU_ATLAS_HEIGHT;
  float u1 = (float)(src.x + src.w) / (float)MU_ATLAS_WIDTH;
  float v1 = (float)(src.y + src.h) / (float)MU_ATLAS_HEIGHT;

  float x0 = (float)dst.x;
  float y0 = (float)dst.y;
  float x1 = (float)(dst.x + dst.w);
  float y1 = (float)(dst.y + dst.h);

  sgl_c4b(color.r, color.g, color.b, color.a);
  sgl_v2f_t2f(x0, y0, u0, v0);
  sgl_v2f_t2f(x1, y0, u1, v0);
  sgl_v2f_t2f(x1, y1, u1, v1);
  sgl_v2f_t2f(x0, y1, u0, v1);

  sgl_end();
}

void microui_begin() { mu_begin(g_mui_state.ctx); }

void microui_end_and_present() {
  bool ok = false;
  if (g_mui_state.ctx->container_stack.idx != 0) {
    fatal_error("microui container stack is not empty");
  } else if (g_mui_state.ctx->clip_stack.idx != 0) {
    fatal_error("microui clip stack is not empty");
  } else if (g_mui_state.ctx->id_stack.idx != 0) {
    fatal_error("microui id stack is not empty");
  } else if (g_mui_state.ctx->layout_stack.idx != 0) {
    fatal_error("microui layout stack is not empty");
  } else {
    ok = true;
  }

  if (!ok) {
    return;
  }

  mu_end(g_mui_state.ctx);

  sgl_enable_texture();
  sgl_texture({g_mui_state.atlas}, {});

  {
    mu_Command *cmd = 0;
    while (mu_next_command(g_mui_state.ctx, &cmd)) {
      switch (cmd->type) {
      case MU_COMMAND_TEXT: {
        mu_Rect dst = {cmd->text.pos.x, cmd->text.pos.y, 0, 0};
        for (const char *p = cmd->text.str; *p; p++) {
          mu_Rect src = mu_atlas_lookup(MU_ATLAS_FONT + (unsigned char)*p);
          dst.w = src.w;
          dst.h = src.h;
          mu_push_quad(dst, src, cmd->text.color);
          dst.x += dst.w;
        }
        break;
      }
      case MU_COMMAND_RECT: {
        mu_push_quad(cmd->rect.rect, mu_atlas_lookup(MU_ATLAS_WHITE),
                     cmd->rect.color);
        break;
      }
      case MU_COMMAND_ICON: {
        mu_Rect rect = cmd->icon.rect;
        mu_Rect src = mu_atlas_lookup(cmd->icon.id);

        int x = rect.x + (rect.w - src.w) / 2;
        int y = rect.y + (rect.h - src.h) / 2;
        mu_push_quad(mu_rect(x, y, src.w, src.h), src, cmd->icon.color);
        break;
      }
      case MU_COMMAND_CLIP: {
        mu_Rect rect = cmd->clip.rect;
        sgl_scissor_rect(rect.x, rect.y, rect.w, rect.h, true);
        break;
      }
      default: break;
      }
    }
  }
}

mu_Rect lua_mu_check_rect(lua_State *L, i32 arg) {
  mu_Rect rect = {};
  rect.x = luax_number_field(L, arg, "x");
  rect.y = luax_number_field(L, arg, "y");
  rect.w = luax_number_field(L, arg, "w");
  rect.h = luax_number_field(L, arg, "h");
  return rect;
}

void lua_mu_rect_push(lua_State *L, mu_Rect rect) {
  lua_createtable(L, 0, 4);
  luax_set_number_field(L, "x", rect.x);
  luax_set_number_field(L, "y", rect.y);
  luax_set_number_field(L, "w", rect.w);
  luax_set_number_field(L, "h", rect.h);
}

mu_Color lua_mu_check_color(lua_State *L, i32 arg) {
  mu_Color color = {};
  color.r = luax_number_field(L, arg, "r");
  color.g = luax_number_field(L, arg, "g");
  color.b = luax_number_field(L, arg, "b");
  color.a = luax_number_field(L, arg, "a");
  return color;
}

void lua_mu_set_ref(lua_State *L, MUIRef *ref, i32 arg) {
  i32 type = lua_type(L, arg);
  switch (type) {
  case LUA_TBOOLEAN:
    ref->kind = MUIRefKind_Boolean;
    ref->boolean = lua_toboolean(L, arg);
    break;
  case LUA_TNUMBER:
    ref->kind = MUIRefKind_Real;
    ref->real = luaL_checknumber(L, arg);
    break;
  case LUA_TSTRING: {
    ref->kind = MUIRefKind_String;
    String s = luax_check_string(L, arg);
    if (s.len > array_size(ref->string) - 1) {
      s.len = array_size(ref->string) - 1;
    }
    memcpy(ref->string, s.data, s.len);
    ref->string[s.len] = '\0';
    break;
  }
  default: ref->kind = MUIRefKind_Nil;
  }
}

MUIRef *lua_mu_check_ref(lua_State *L, i32 arg, MUIRefKind kind) {
  MUIRef *ref = *(MUIRef **)luaL_checkudata(L, arg, "mt_mu_ref");

  if (ref->kind != kind) {
    memset(ref, 0, sizeof(MUIRef));
    ref->kind = kind;
  }

  return ref;
}
