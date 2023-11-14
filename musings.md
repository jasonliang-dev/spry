# Musings

Some random text.

## Future

- Shaders
  - If compiled online, how in the name of heck would I do this?
  - If compiled offline, just use `sokol-shdc`? But how would uniforms work?
- Networking
  - HTTP? UDP with enet?
- UI

## `defer`

See https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/.

Spry's C++ code reads more like C than modern C++. One of the C++ features I
ignore is RAII.

I don't want to write:

- A constructor
- A copy constructor
- A copy assignment operator overload
- A move constructor
- A move assignment operator overload

... for each type.

And I also don't want to concern myself with construction and destruction in
the proper places within my arrays and hash maps. I would also need two
different `array_push` functions, one for copy and one for move. And if I
really wanted things to be *generic*&trade; I'd also introduce perfect
forwarding for the places that use these functions.

Sometimes I want to just forget about a buffer I allocated when the scope
exits, and sometimes I don't! `defer` makes destruction obvious, and I like
that.

The one exception to this is `Instrument` in `profile.h`. It uses its
constructor and destructor to write trace events.

## Memory

- General allocation with malloc/free
- Debug allocation through doubly linked list
- Arena allocation with 4kb (or larger) chunks in singly linked list
  - Types that use arenas:
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
- Queue backed by a ring buffer

STL exists, yes, but I would like to compile my program in a reasonable time
please. Also, the STL containers hits debug performance pretty hard.

## Algorithms

- JSON parsing (recursive descent)
- A\* pathfinding

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

For debug builds, or if `USE_PROFILER` was defined when building, a
`profile.json` file is produced while running the program. This file can be
loaded in a profiler such as `chrome://tracing/` or
https://gravitymoth.com/spall/.

## FPS limiting

This is done by calling `Sleep` but the timings are uhh, not good. To make
sleep more accurate, call `Sleep` for a little less than the target time,
then spin loop the rest.

The better thing to do is VSync, but there's noticeable input latency.

## `os_file_modtime`

Hot reloading is done by comparing file modification time. This performs
decent on Linux, but it takes about 100-200us per file on Windows with a
mid-range Windows PC. 100 microseconds adds up fast with a decent number of
files.

~~Eventually I want to do something different.~~
- ~~Maybe use `ReadDirectoryChangesW`?~~
- ~~Compare mod time in a different thread?~~

Update: Hot reloading is now done on a separate thread.

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
