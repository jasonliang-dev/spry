# Hot Reloading

Spry has the ablity to hot reload recently changed Lua scripts, Aseprite
files, LDtk files, and images. While Spry is running, you can update a file
and the changes will be reflected without having to restart your program.

<video autoplay muted loop class="w-100 mw7 center db mb2">
  <source src="static/assets/hot-lua.webm" type="video/webm">
</video>

<video autoplay muted loop class="w-100 mw7 center db mb2">
  <source src="static/assets/hot-png.webm" type="video/webm">
</video>

<video autoplay muted loop class="w-100 mw7 center db mb2">
  <source src="static/assets/hot-ldtk.webm" type="video/webm">
</video>

Hot reloading Lua scripts will not update any references or callbacks. For
example, If you change a callback function in a class constructor, old
instances will still hold onto the old function.

```lua
function Enemy:new()
  self.timer = interval(0.1, function()
    -- update this function and save the file.
    -- changes are reflected for new Enemy instances only.
  end)
end
```

Hot reloads are done by checking the modified time of each loaded file. By
default, this is done every 1/10th of a second. To change this, set
`reload_interval` to a different value in `spry.conf`:

```lua
function spry.conf(t)
  t.reload_interval = 0.5 -- reload every half second
end
```

You can even set the reload interval to 0 if you want to poll for changes as
fast as possible, but it increases the chance that Spry is reading the file
you want to write to, and it wastes disk usage.

To disable hot reloading, set `hot_reload` to `false` in `spry.conf`.

```lua
function spry.conf(t)
  t.hot_reload = false -- disable hot reloading
end
```

Hot reloading is not supported for web builds.
