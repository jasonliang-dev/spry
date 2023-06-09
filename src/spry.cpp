#define MAKE_LIB
#include "deps/lua/onelua.c"

#define CUTE_ASEPRITE_ALLOC(size, ctx) mem_alloc(size)
#define CUTE_ASEPRITE_FREE(mem, ctx) mem_free(mem)

#define _TINYDIR_MALLOC(_size) mem_alloc(_size)
#define _TINYDIR_FREE(_ptr) mem_free(_ptr)

#include "app.h"

// extern(prelude.h)
#ifdef DEBUG
Allocator g_allocator = debug_allocator();
#endif

#ifdef RELEASE
Allocator g_allocator = heap_allocator();
#endif

// extern(app.h)
App *g_app;

#include "api.cpp"
#include "archive.cpp"
#include "atlas.cpp"
#include "audio.cpp"
#include "draw.cpp"
#include "font.cpp"
#include "image.cpp"
#include "luax.cpp"
#include "scanner.cpp"
#include "sprite.cpp"
#include "strings.cpp"
#include "tilemap.cpp"

#include "main.cpp"

#include "deps/box2d/collision/b2_broad_phase.cpp"
#include "deps/box2d/collision/b2_chain_shape.cpp"
#include "deps/box2d/collision/b2_circle_shape.cpp"
#include "deps/box2d/collision/b2_collide_circle.cpp"
#include "deps/box2d/collision/b2_collide_edge.cpp"
#include "deps/box2d/collision/b2_collide_polygon.cpp"
#include "deps/box2d/collision/b2_collision.cpp"
#include "deps/box2d/collision/b2_distance.cpp"
#include "deps/box2d/collision/b2_dynamic_tree.cpp"
#include "deps/box2d/collision/b2_edge_shape.cpp"
#include "deps/box2d/collision/b2_polygon_shape.cpp"
#include "deps/box2d/collision/b2_time_of_impact.cpp"
#include "deps/box2d/common/b2_block_allocator.cpp"
#include "deps/box2d/common/b2_draw.cpp"
#include "deps/box2d/common/b2_math.cpp"
#include "deps/box2d/common/b2_settings.cpp"
#include "deps/box2d/common/b2_stack_allocator.cpp"
#include "deps/box2d/common/b2_timer.cpp"
#include "deps/box2d/dynamics/b2_body.cpp"
#include "deps/box2d/dynamics/b2_chain_circle_contact.cpp"
#include "deps/box2d/dynamics/b2_chain_polygon_contact.cpp"
#include "deps/box2d/dynamics/b2_circle_contact.cpp"
#include "deps/box2d/dynamics/b2_contact.cpp"
#include "deps/box2d/dynamics/b2_contact_manager.cpp"
#include "deps/box2d/dynamics/b2_contact_solver.cpp"
#include "deps/box2d/dynamics/b2_distance_joint.cpp"
#include "deps/box2d/dynamics/b2_edge_circle_contact.cpp"
#include "deps/box2d/dynamics/b2_edge_polygon_contact.cpp"
#include "deps/box2d/dynamics/b2_fixture.cpp"
#include "deps/box2d/dynamics/b2_friction_joint.cpp"
#include "deps/box2d/dynamics/b2_gear_joint.cpp"
#include "deps/box2d/dynamics/b2_island.cpp"
#include "deps/box2d/dynamics/b2_joint.cpp"
#include "deps/box2d/dynamics/b2_motor_joint.cpp"
#include "deps/box2d/dynamics/b2_mouse_joint.cpp"
#include "deps/box2d/dynamics/b2_polygon_circle_contact.cpp"
#include "deps/box2d/dynamics/b2_polygon_contact.cpp"
#include "deps/box2d/dynamics/b2_prismatic_joint.cpp"
#include "deps/box2d/dynamics/b2_pulley_joint.cpp"
#include "deps/box2d/dynamics/b2_revolute_joint.cpp"
#include "deps/box2d/dynamics/b2_weld_joint.cpp"
#include "deps/box2d/dynamics/b2_wheel_joint.cpp"
#include "deps/box2d/dynamics/b2_world.cpp"
#include "deps/box2d/dynamics/b2_world_callbacks.cpp"
#include "deps/box2d/rope/b2_rope.cpp"

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "deps/cute_aseprite.h"

#include "deps/miniz.c"

#define SOKOL_IMPL
#if defined(_WIN32)
#define SOKOL_D3D11
#ifdef DEBUG
#define SOKOL_WIN32_FORCE_MAIN
#endif
#elif defined(__linux__)
#define SOKOL_GLCORE33
#elif defined(__EMSCRIPTEN__)
#define SOKOL_GLES2
#endif
#include "deps/sokol_app.h"
#include "deps/sokol_audio.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "deps/sokol_glue.h"
#include "deps/sokol_log.h"
#include "deps/sokol_time.h"

#define STB_IMAGE_IMPLEMENTATION
#include "deps/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "deps/stb_truetype.h"

#undef STB_VORBIS_HEADER_ONLY
#include "deps/stb_vorbis.c"
