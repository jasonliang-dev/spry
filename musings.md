# Musings

Some random text.

## Future

- Shaders
  - If compiled online, how in the name of heck would I do this?
  - If compiled offline, just use `sokol-shdc`? But how would uniforms work?
- Networking
  - HTTP? UDP with enet? LuaSocket?
- MobDebug (requires LuaSocket)
- UI

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

There's a few places where constructors and destructors are used:

- `Allocator` in `prelude.h`. It pairs well with inheritance and virtual
  functions. There's only one `Allocator`. No copies are allowed. The
  constructor/destructor is called via `new`/`delete`.
- `FileSystem` in `vfs.cpp` is similar to `Allocator`, being a abstract base
  type that can't be copied. It uses `mem_alloc`/`mem_free` instead of
  `new`/`delete`. It uses placement new and the destructor is explicitly
  called.
- `Instrument` in `profile.h` uses its constructor and destructor to write
  trace events.

To some (many?), this isn't considered proper RAII usage since these types are
constructed/destructed explicitly.

The rationale for doing this with `Allocator` and `FileSystem` is that I
wanted vtables for these types, and I found vtables hard to read when written
in the way you would in C. For `Instrument`, it was out of convenience.

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
- Unbounded queue backed by a ring buffer

STL exists, yes, but I would like to compile my program in a reasonable time
please. Also, the STL containers hits debug performance pretty hard.

## Algorithms

- `profile.cpp` - Producer/consumer with unbounded queue
- `json.cpp` -  recursive descent parsing
- `tilemap.cpp` - A\* pathfinding

Since tilemaps are grids, and it's not uncommon for graph nodes to have the
same costs for traversal, JPS is a good fit, and I'd like to put it in this
project at some point.

## SIMD

The `Vector4` and `Matrix4` types in `algebra.h` use SSE intrinsics. The
renderer in `draw.cpp` also takes advantage of SIMD.

I barely know how SIMD works.

## Archive

The `Archive` type acts like a virtual file system. You *could* read files
directly (`io.open` for Lua, `fopen` for C/C++), but don't. Spry has the
ability to load data from a zip archive, where file paths are treated as if
they were located in a regular folder. This is also the reason why the
`require` function in Lua was changed.

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

## Music

When audio files are loaded into memory, the data is decoded into raw samples
on the spot. This is fine for short sounds, but not good for music. A 1m50s
`.ogg` file takes 200ms on my machine to decode (475ms for debug builds).
