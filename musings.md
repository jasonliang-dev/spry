# Musings

Some random text.

## Future

- Shaders
  - If compiled online, how in the name of heck would I do this? Just write
    multiple versions per backend?
  - If compiled offline, just use `sokol-shdc`? But how would uniforms work?
- Async file loading
  - Promises? Threads/channels? Callbacks? A combination of the three?
- Android support
- Add debugger breakpoints on error. Is it even possible?
- Filesystem API
- More Box2D

## `defer` and RAII

See https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/.

Spry's C++ code reads more like C than modern C++. One of the C++ features I
(mostly) ignore is RAII.

I don't want to write a:

- constructor
- copy constructor
- copy assignment operator
- move constructor
- move assignment operator

... for each type.

And I also don't want to concern myself with construction and destruction in
the proper places within my arrays and hash maps. I would also need two
different `array_push` functions, one for copy and one for move. And if I
really wanted things to be *generic*&trade; I'd also introduce perfect
forwarding for the places that use these functions.

Sometimes I want to just forget about a buffer I allocated when the scope
exits, and sometimes I don't! `defer` makes destruction obvious, and I like
that.

There's a few types that use destructors, but not directly. Instead these
types are used through macros:

- The macro `defer` uses `Defer` (`prelude.h`), which runs a lambda function
  in its destructor.
- The macros `PROFILE_FUNC` and `PROFILE_BLOCK` uses `Instrument`
  (`profile.h`), which produces trace events in its constructor and destructor.

The sync primitives (mutex, condition variables, etc), also uses RAII. It's a
good fit for these types since copies and moves are not allowed.
Non-copyable/moveable types removes a lot of the complexity that comes with
RAII.

## Memory

- General allocation with malloc/free
- Debug allocation through doubly linked list
- Arena allocation with 4kb (or larger) chunks in singly linked list
  - Sprites use it for arrays
  - JSON parser uses it for arrays/objects (except JSONArray is actually a
    linked list)
  - Tilemap uses it for arrays and strings

## Data structures

- Strings
  - String view
  - String builder
  - UTF-8
- Dynamic array
- Open addressing hash map
- Min heap priority queue
- Thread-safe unbounded queue
- Go style channel

STL exists, yes, but I would like to compile my program in a reasonable time
please. Also, the STL containers hits debug performance pretty hard.

## Algorithms

- `profile.cpp` - Producer/consumer

  The queue lives in `chan.h` and uses a mutex + condition variable. I
  replaced it with semaphores, but it was slower? This change was reverted.

  `json_parse` timings (dungeon example project) on a decade old ThinkPad:

  - mutex + condition variable: ~40ms debug, ~7ms release (woah one order of magnitude)
  - semaphores + atomics: ~70ms debug, ~70ms release
  - semaphores + mutex: ~80ms debug, ~80ms release

- `json.cpp` -  recursive descent parsing
- `tilemap.cpp` - A\* pathfinding

Since tilemaps are grids, and it's not uncommon for graph nodes to have the
same costs for traversal, JPS is a good fit, and I'd like to put it in this
project at some point.

## SIMD

The `Vector4` and `Matrix4` types in `algebra.h` use SSE intrinsics. The
renderer in `draw.cpp` also takes advantage of SIMD.

I barely know how SIMD works.

## Virtual File System

You *could* read files directly (`io.open` for Lua, `fopen` for C/C++), but
don't. Spry has the ability to load data from a zip archive, where file paths
are treated as if they were located in a regular folder. This is also the
reason why the `require` function in Lua was changed.

## Profiling

For debug builds, or if `USE_PROFILER` was defined during compilation, a
`profile.json` file is produced while running the program. This file can be
loaded in a profiler such as `chrome://tracing/` or
https://gravitymoth.com/spall/.

## FPS limiting

This is done by calling `Sleep` but the timings are uhh, not good. To make
sleep more accurate, call `Sleep` for a little less than the target time,
then spin loop the rest.

The better thing to do is VSync, but there's noticeable input latency.

## Hot reloading classes

The Lua `class` function takes advantage of the fact that it creates global
variables.

```lua
function class(name)
  -- ...
  local cls = {}
  function cls:__index(key)
    return rawget(_G, name)[key]
  end
  -- ...
end
```

This `__index` metamethod allows for hot reloading with classes.
