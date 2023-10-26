<?php article(function () { ?>

## Hot Reloading

Spry has the ablity to hot reload recently changed Lua scripts, Aseprite
files, LDtk files, and images. While Spry is running, you can update a file
and the changes will be reflected without having to restart your program.

<video autoplay muted loop class="w-100 mw7 center db mb2">
  <source src="static/hot-lua.webm" type="video/webm">
</video>

<video autoplay muted loop class="w-100 mw7 center db mb2">
  <source src="static/hot-png.webm" type="video/webm">
</video>

<video autoplay muted loop class="w-100 mw7 center db mb2">
  <source src="static/hot-ldtk.webm" type="video/webm">
</video>

Hot reloading Lua scripts will not update any references or callbacks. For
example, if you modify a class that has a draw method, only newly created
instances of the class will use the new version of the method.

```lua
class 'Enemy'

function Enemy:draw()
  -- if you update `rot` here, only new enemies will use the updated value.
  -- existing enemies use the previous value.
  local rot = math.pi

  self.img:draw(self.x, self.y, rot)
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

You can even set the reload interval to 0 if you want to poll for changes
every frame.

To disable hot reloading, set `hot_reload` to `false` in `spry.conf`.

```lua
function spry.conf(t)
  t.hot_reload = false -- disable hot reloading
end
```

Hot reloading is not supported for web builds.

<?php });

footer("mw7");
