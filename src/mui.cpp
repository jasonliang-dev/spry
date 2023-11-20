#include "mui.h"
#include "deps/microui_atlas.inl"
#include "deps/sokol_app.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "prelude.h"

struct MicrouiState {
  mu_Context *ctx;
  u32 atlas;
};

static MicrouiState g_mui_state;

mu_Context *mui_ctx() { return g_mui_state.ctx; }

void mui_init() {
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

void mui_trash() { mem_free(g_mui_state.ctx); }

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

void mui_sokol_event(const sapp_event *e) {
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

static void write_log(const char *text) { printf("%s\n", text); }

static void test_window(mu_Context *ctx) {
  /* do window */
  if (mu_begin_window_ex(ctx, "Demo Window", mu_rect(40, 40, 300, 450),
                         MU_OPT_NOCLOSE)) {
    mu_Container *win = mu_get_current_container(ctx);
    win->rect.w = mu_max(win->rect.w, 240);
    win->rect.h = mu_max(win->rect.h, 300);

    /* window info */
    if (mu_header(ctx, "Window Info")) {
      mu_Container *win = mu_get_current_container(ctx);
      char buf[64];
      {
        int w[] = {54, -1};
        mu_layout_row(ctx, 2, w, 0);
      }
      mu_label(ctx, "Position:");
      sprintf(buf, "%d, %d", win->rect.x, win->rect.y);
      mu_label(ctx, buf);
      mu_label(ctx, "Size:");
      sprintf(buf, "%d, %d", win->rect.w, win->rect.h);
      mu_label(ctx, buf);
    }

    /* labels + buttons */
    if (mu_header_ex(ctx, "Test Buttons", MU_OPT_EXPANDED)) {
      {
        int w[] = {86, -110, -1};
        mu_layout_row(ctx, 3, w, 0);
      }
      mu_label(ctx, "Test buttons 1:");
      if (mu_button(ctx, "Button 1")) {
        write_log("Pressed button 1");
      }
      if (mu_button(ctx, "Button 2")) {
        write_log("Pressed button 2");
      }
      mu_label(ctx, "Test buttons 2:");
      if (mu_button(ctx, "Button 3")) {
        write_log("Pressed button 3");
      }
      if (mu_button(ctx, "Popup")) {
        mu_open_popup(ctx, "Test Popup");
      }
      if (mu_begin_popup(ctx, "Test Popup")) {
        mu_button(ctx, "Hello");
        mu_button(ctx, "World");
        mu_end_popup(ctx);
      }
    }

    /* tree */
    if (mu_header_ex(ctx, "Tree and Text", MU_OPT_EXPANDED)) {
      {
        int w[] = {140, -1};
        mu_layout_row(ctx, 2, w, 0);
      }
      mu_layout_begin_column(ctx);
      if (mu_begin_treenode(ctx, "Test 1")) {
        if (mu_begin_treenode(ctx, "Test 1a")) {
          mu_label(ctx, "Hello");
          mu_label(ctx, "world");
          mu_end_treenode(ctx);
        }
        if (mu_begin_treenode(ctx, "Test 1b")) {
          if (mu_button(ctx, "Button 1")) {
            write_log("Pressed button 1");
          }
          if (mu_button(ctx, "Button 2")) {
            write_log("Pressed button 2");
          }
          mu_end_treenode(ctx);
        }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 2")) {
        {
          int w[] = {54, 54};
          mu_layout_row(ctx, 2, w, 0);
        }
        if (mu_button(ctx, "Button 3")) {
          write_log("Pressed button 3");
        }
        if (mu_button(ctx, "Button 4")) {
          write_log("Pressed button 4");
        }
        if (mu_button(ctx, "Button 5")) {
          write_log("Pressed button 5");
        }
        if (mu_button(ctx, "Button 6")) {
          write_log("Pressed button 6");
        }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 3")) {
        static int checks[3] = {1, 0, 1};
        mu_checkbox(ctx, "Checkbox 1", &checks[0]);
        mu_checkbox(ctx, "Checkbox 2", &checks[1]);
        mu_checkbox(ctx, "Checkbox 3", &checks[2]);
        mu_end_treenode(ctx);
      }
      mu_layout_end_column(ctx);

      mu_layout_begin_column(ctx);
      {
        int w[] = {-1};
        mu_layout_row(ctx, 1, w, 0);
      }
      mu_text(
          ctx,
          "Lorem ipsum dolor sit amet, consectetur adipiscing "
          "elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
          "ipsum, eu varius magna felis a nulla.");
      mu_layout_end_column(ctx);
    }

    static float bg[3] = {90, 95, 100};

    /* background color sliders */
    if (mu_header_ex(ctx, "Background Color", MU_OPT_EXPANDED)) {
      {
        int w[] = {-78, -1};
        mu_layout_row(ctx, 2, w, 74);
      }
      /* sliders */
      mu_layout_begin_column(ctx);
      {
        int w[] = {46, -1};
        mu_layout_row(ctx, 2, w, 0);
      }
      mu_label(ctx, "Red:");
      mu_slider(ctx, &bg[0], 0, 255);
      mu_label(ctx, "Green:");
      mu_slider(ctx, &bg[1], 0, 255);
      mu_label(ctx, "Blue:");
      mu_slider(ctx, &bg[2], 0, 255);
      mu_layout_end_column(ctx);
      /* color preview */
      mu_Rect r = mu_layout_next(ctx);
      mu_draw_rect(ctx, r, mu_color(bg[0], bg[1], bg[2], 255));
      char buf[32];
      sprintf(buf, "#%02X%02X%02X", (int)bg[0], (int)bg[1], (int)bg[2]);
      mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
    }

    mu_end_window(ctx);
  }
}

void mui_test_window() { test_window(g_mui_state.ctx); }

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

void mui_draw() {
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
