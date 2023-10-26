<?php article(function () { ?>

# UFO Game

<video controls class="w-100 mw7 center db">
  <source src="static/ufo-demo.webm" type="video/webm">
</video>

We'll be creating a little demo that sees a UFO move around the screen using
the keyboard. Create a new folder, then add the following code to a new
`main.lua` file:

```lua
function spry.conf(t)
  t.window_title = 'UFO Demo'
end
```

[kenney.nl](https://kenney.nl/assets/alien-ufo-pack) provides free
(public domain) game assets. One of the asset packs includes
[a green UFO](static/green-ufo.png). Add it to the game folder, and load it
using `spry.image_load`.

```lua
function spry.start()
  ufo = spry.image_load 'green-ufo.png'
end
```

## Player Class

We'll create a new file called `player.lua` with the following contents:

```lua
class 'Player'

function Player:new()
  self.x = spry.window_width() / 2
  self.y = spry.window_height() / 2
end

function Player:draw()
  ufo:draw(self.x, self.y)
end
```

Inside `spry.start`, We'll create a new player back in the `main.lua` file:

```lua
function spry.start()
  ufo = spry.image_load 'green-ufo.png'
  player = Player()
end
```

`Player()` calls the `Player:new` method. The player's x and y position is
one half of the window's width and height. In other words, we're setting the
player's position to the center of the screen.

`Player:draw` uses the ufo image that we load during program startup in
`spry.start`. The image is drawn with the player's x and y position.

Let's create `spry.frame` in `main.lua`. In this function, we'll draw the
player:

```lua
function spry.frame(dt)
  player:draw()
end
```

You do not need to include the newly created file inside `main.lua`. All Lua
scripts in your project folder are loaded on startup. Once you run your game,
you should see a ufo on the screen:

```plaintext
spry.exe my_folder
```

## Keyboard Input

We'll use keyboard input to move the player. Create a new `Player:update`
method:

```lua
function Player:update(dt)
  local vx, vy = 0, 0

  if spry.key_down 'w' then vy = vy - 1 end
  if spry.key_down 's' then vy = vy + 1 end
  if spry.key_down 'a' then vx = vx - 1 end
  if spry.key_down 'd' then vx = vx + 1 end
end
```

`vx` and `vy` stands for velocity x, and velocity y. These variables represent
the player's horizonal and vertical speed. When the user presses any of the
WASD keys, the velocity changes appropriately. In the case of the D key, `vx`
is set to itself, plus 1, representing movement towards the right side of the
screen.

Now we'll move the player by setting the x and y position. Put the following
code inside `Player:update`, just before the function ends:

```lua
local move_speed = 200
self.x = self.x + vx * move_speed * dt
self.y = self.y + vy * move_speed * dt
```

Whenever we move something, we need to make sure the movement is framerate
independent. This is done by multiplying movement speed by delta time `dt`
(the time spend between this frame and last frame). We'll pass `dt` to the
player's update method through `spry.update`:

```lua
function spry.frame(dt)
  player:update(dt)
  player:draw()
end
```

Run the game again and try moving the UFO using the WASD keys.

## Image Origin

When drawing the UFO, the image origin starts at the top-left. If you want the
player to be exactly centered, change `Player:draw`:

```lua
function Player:draw()
  local rotation = 0

  -- scale
  local sx = 1
  local sy = 1

  -- image origin
  local ox = ufo:width() / 2
  local oy = ufo:height() / 2

  ufo:draw(self.x, self.y, rotation, sx, sy, ox, oy)
end
```

Here, we're adding additional info to `ufo:draw`. To set the image origin to
the center of the image, get the image dimensions with `ufo:width` and
`ufo:height`, and divide by 2. In order to set the image origin, we also need
to explicitly set the image's rotation, horizontal scale, and vertical scale
first.

## Movement Fix

You might notice that moving diagonally is a little faster compared to
vertical or horizonal movement. This is because vertical and horizontal
movement has a speed of 200, but using both vertical and horizontal speeds
together results in a speed of around 282 (square root of 200<sup>2</sup> +
200<sup>2</sup>). We need to normalize `vx` and `vy` so that the combined
speed of both `vx` and `vy` is always 1.

After setting keyboard input, but before moving the player, add the following line:

```lua
vx, vy = normalize(vx, vy)
```

Run the game again, and you'll see that the player moves at the same speed
regardless of direction.

<?php });

footer("mw7");
