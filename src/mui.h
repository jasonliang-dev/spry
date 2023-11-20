#pragma once

#include "deps/microui.h"

struct sapp_event;

mu_Context *mui_ctx();
void mui_init();
void mui_trash();
void mui_sokol_event(const sapp_event *e);
void mui_draw();
void mui_test_window();