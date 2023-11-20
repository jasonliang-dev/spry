#pragma once

#include "deps/microui.h"
#include "prelude.h"

struct sapp_event;

mu_Context *microui_ctx();
void microui_init();
void microui_trash();
void microui_sokol_event(const sapp_event *e);
void microui_begin();
void microui_end_and_present();

struct lua_State;
mu_Rect lua_mu_check_rect(lua_State *L, i32 arg);
void lua_mu_rect_push(lua_State *L, mu_Rect rect);
mu_Color lua_mu_check_color(lua_State *L, i32 arg);

enum MUIRefKind : i32 {
  MUIRefKind_Nil,
  MUIRefKind_Boolean,
  MUIRefKind_Real,
  MUIRefKind_String,
};

struct MUIRef {
  MUIRefKind kind;
  union {
    int boolean;
    mu_Real real;
    char string[512];
  };
};

void lua_mu_set_ref(lua_State *L, MUIRef *ref, i32 arg);
MUIRef *lua_mu_check_ref(lua_State *L, i32 arg, MUIRefKind kind);
