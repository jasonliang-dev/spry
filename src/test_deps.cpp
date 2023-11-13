#define CUTE_SYNC_IMPLEMENTATION
#if defined(_WIN32)
#define CUTE_SYNC_WINDOWS
#else
#define CUTE_SYNC_POSIX
#endif
#include "deps/cute_sync.h"
