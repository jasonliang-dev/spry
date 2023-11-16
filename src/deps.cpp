#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

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

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "deps/cute_aseprite.h"

#define CUTE_SYNC_IMPLEMENTATION
#if defined(_WIN32)
#define CUTE_SYNC_WINDOWS
#else
#define CUTE_SYNC_POSIX
#endif
#include "deps/cute_sync.h"

#include "deps/luaalloc.c"

#define MAKE_LIB
#include "deps/lua/onelua.c"

#include "deps/miniz.c"

#define SOKOL_IMPL
#if defined(_WIN32)
#define SOKOL_D3D11
#define SOKOL_WIN32_FORCE_MAIN
#ifdef DEBUG
#define SOKOL_DEBUG
#endif
#elif defined(__linux__)
#define SOKOL_GLCORE33
#elif defined(__EMSCRIPTEN__)
#define SOKOL_GLES3
#endif
#include "deps/sokol_app.h"
#include "deps/sokol_gfx.h"
#include "deps/sokol_gl.h"
#include "deps/sokol_glue.h"
#include "deps/sokol_log.h"
#include "deps/sokol_time.h"

#define STB_IMAGE_IMPLEMENTATION
#include "deps/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "deps/stb_truetype.h"

#define STB_VORBIS_HEADER_ONLY
#include "deps/stb_vorbis.c"

#define MA_NO_GENERATION
#define MA_NO_RESOURCE_MANAGER
#define MINIAUDIO_IMPLEMENTATION
#include "deps/miniaudio.h"

#undef STB_VORBIS_HEADER_ONLY
#include "deps/stb_vorbis.c"
