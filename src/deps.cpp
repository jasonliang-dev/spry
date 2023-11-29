#ifdef _WIN32
#pragma comment(lib, "ws2_32")
#endif

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "deps/cute_aseprite.h"

#include "deps/luaalloc.c"

extern "C" {
#define MAKE_LIB
#include "deps/lua/onelua.c"
}

#undef RELATIVE
#undef ABSOLUTE
#include "deps/microui.c"

#include "deps/miniz.c"

#define SOKOL_IMPL
#if defined(_WIN32)
#define SOKOL_D3D11
#define SOKOL_WIN32_FORCE_MAIN
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

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI
#define MA_ENABLE_ALSA
#define MA_ENABLE_WEBAUDIO
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION
#include "deps/miniaudio.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#undef STB_VORBIS_HEADER_ONLY
#include "deps/stb_vorbis.c"
