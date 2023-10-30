<?php

$draw_description = [
  "x" => ["number", "The x position to draw at.", 0],
  "y" => ["number", "The y position to draw at.", 0],
  "r" => ["number", "The rotation angle in radians.", 0],
  "sx" => ["number", "The x scale factor.", 1],
  "sy" => ["number", "The y scale factor.", 1],
  "ox" => ["number", "The x origin offset.", 0],
  "oy" => ["number", "The y origin offset.", 0],
];

$rect_description = [
  "x" => ["number", "The x position to draw at.", 0],
  "y" => ["number", "The y position to draw at.", 0],
  "w" => ["number", "The width of the rectangle.", 0],
  "h" => ["number", "The height of the rectangle.", 0],
  "rotation" => ["number", "The rotation angle in radians.", 0],
  "sx" => ["number", "The x scale factor.", 1],
  "sy" => ["number", "The y scale factor.", 1],
  "ox" => ["number", "The x origin offset.", 0],
  "oy" => ["number", "The y origin offset.", 0],
];

$b2body_def = [
  "t" => ["table", "Body definition."],
  " .x" => ["number", "The x position of the physics body."],
  " .y" => ["number", "The y position of the physics body."],
  " .vx" => ["number", "The x velocity of the physics body.", 0],
  " .vy" => ["number", "The y velocity of the physics body.", 0],
  " .angle" => ["number", "The angle of the physics body in radians.", 0],
  " .linear_damping" => ["number", "The linear damping of the physics body.", 0],
  " .fixed_rotation" => ["boolean", "If true, prevent the physics body from rotating.", "false"],
  " .udata" => ["string | number", "Custom user data for this body.", "nil"],
];

$b2fixture_def = [
  " .sensor" => ["boolean", "True if the fixture should be a sensor.", "false"],
  " .density" => ["number", "The fixture's density.", 1],
  " .friction" => ["number", "The fixture's friction.", 0.2],
  " .restitution" => ["number", "The fixture's restitution.", 0],
  " .udata" => ["string | number", "Custom user data for this fixture.", "nil"],
  " .begin_contact" => ["function", "Run a callback function when this fixture touches another.", "nil"],
  " .end_contact" => ["function", "Run a callback function when this fixture stops touching another.", "nil"],
];

$api_reference = [
  "Callback Functions" => [
    "spry.conf" => [
      "desc" => "Define this callback function to configure various program options.",
      "example" => "
        function spry.conf(t)
          t.window_width = 800
          t.window_height = 600
        end
      ",
      "args" => [
        "t" => ["table", "The table to edit options with."],
        " .console_attach" => ["boolean", "Windows only. If true, attach a console to the program.", "false"],
        " .hot_reload" => ["boolean", "Enable/disable hot reloading of scripts and assets.", "true"],
        " .reload_interval" => ["number", "The time in seconds to update files for hot reloading.", 0.1],
        " .swap_interval" => ["number", "Set the swap interval. Typically 1 for VSync, or 0 for no VSync.", 1],
        " .window_width" => ["number", "The window width.", 800],
        " .window_height" => ["number", "The window height.", 600],
        " .window_title" => ["string", "The window title.", "'Spry'"],
      ],
      "return" => false,
    ],
    "spry.start" => [
      "desc" => "
        Define this callback function to perform initialization work, such as
        loading assets, or creating objects.
      ",
      "example" => "
        function spry.start()
          img = spry.image_load 'tile.png'
          font = spry.font_load 'roboto.ttf'
        end
      ",
      "args" => [],
      "return" => false,
    ],
    "spry.frame" => [
      "desc" => "
        Define this callback function to update program state and to draw
        things to the screen. This function is called every frame. This
        function typically gets called 60 times a second if VSync is on.
      ",
      "example" => "
        function spry.start()
          img = spry.image_load 'tile.png'
          x = 0
        end

        function spry.frame(dt)
          x = x + dt * 4
          img:draw(x, 100)
        end
      ",
      "args" => [
        "dt" => ["number", "Delta time."]
      ],
      "return" => false,
    ],
  ],
  "Core Functions" => [
    "spry.quit" => [
      "desc" => "Exit the program.",
      "example" => "
        if player_wants_to_quit then
          spry.quit()
        end
      ",
      "args" => [],
      "return" => false,
    ],
    "spry.platform" => [
      "desc" => "Returns the platform as a string. One of `html5`, `windows`, or `linux`.",
      "example" => "
        if spry.platform() ~= 'html5' then
          display_quit_button()
        end
      ",
      "args" => [],
      "return" => "string",
    ],
    "spry.dt" => [
      "desc" => "Returns delta time. The time in seconds between this frame and last frame.",
      "example" => "self.x = self.x + vel_x * spry.dt()",
      "args" => [],
      "return" => "number",
    ],
    "spry.window_width" => [
      "desc" => "Get the width of the application window.",
      "example" => "x = clamp(x, 0, spry.window_width())",
      "args" => [],
      "return" => "number",
    ],
    "spry.window_height" => [
      "desc" => "Get the height of the application window.",
      "example" => "x = clamp(x, 0, spry.window_height())",
      "args" => [],
      "return" => "number",
    ],
    "spry.key_down" => [
      "desc" => "
        Check if a keyboard key is held down since last frame.

        The key can be one of the following strings:

        `space` `'` `,` `-` `.` `/` `0` `1` `2` `3` `4` `5` `6` `7` `8` `9`
        `;` `=` `a` `b` `c` `d` `e` `f` `g` `h` `i` `j` `k` `l` `m` `n` `o`
        `p` `q` `r` `s` `t` `u` `v` `w` `x` `y` `z` `[` `\\` `]` `` ` ``
        `world_1` `world_2` `esc` `enter` `tab` `backspace` `insert` `delete`
        `right` `left` `down` `up` `pg_up` `pg_down` `home` `end` `caps_lock`
        `scroll_lock` `num_lock` `print_screen` `pause` `f1` `f2` `f3` `f4`
        `f5` `f6` `f7` `f8` `f9` `f10` `f11` `f12` `f13` `f14` `f15` `f16`
        `f17` `f18` `f19` `f20` `f21` `f22` `f23` `f24` `f25` `kp0` `kp1`
        `kp2` `kp3` `kp4` `kp5` `kp6` `kp7` `kp8` `kp9` `kp.` `kp/` `kp*`
        `kp-` `kp+` `kp_enter` `kp=` `lshift` `lctrl` `lalt` `lsuper`
        `rshift` `rctrl` `ralt` `rsuper` `menu`
      ",
      "example" => "
        local vy = 0
        if spry.key_down 'w' then vy = vy - 1 end
        if spry.key_down 's' then vy = vy + 1 end
      ",
      "args" => [
        "key" => ["string", "The key to check."]
      ],
      "return" => "boolean",
    ],
    "spry.key_press" => [
      "desc" => "
        Check if a keyboard key was pressed since last frame. The key can be
        any of the strings used in [`spry.key_down`](#spry.key_down).
      ",
      "example" => "
        if spry.key_press 'space' then
          self:jump()
        end
      ",
      "args" => [
        "key" => ["string", "The key to check."]
      ],
      "return" => "boolean",
    ],
    "spry.key_release" => [
      "desc" => "
        Check if a keyboard key was released since last frame. The key can be
        any of the strings used in [`spry.key_down`](#spry.key_down).
      ",
      "example" => "
        if spry.key_up 'e' then
          self:interact_with(nearest_object)
        end
      ",
      "args" => [
        "key" => ["string", "The key to check."]
      ],
      "return" => "boolean",
    ],
    "spry.mouse_down" => [
      "desc" => "
        Check if a mouse button is held down since last frame. Mouse button 0
        is the left button, button 1 is the right button, and button 2 is the
        middle button.
      ",
      "example" => "
        if spry.mouse_down(0) then
          aim_to(mx, my)
        end
      ",
      "args" => [
        "button" => ["number", "The mouse button."]
      ],
      "return" => "boolean",
    ],
    "spry.mouse_click" => [
      "desc" => "
        Check if a mouse button was clicked since last frame. Mouse button 0
        is the left button, button 1 is the right button, and button 2 is the
        middle button.
      ",
      "example" => "
        if spry.mouse_click(0) then
          spawn_block(mx, my)
        end
      ",
      "args" => [
        "button" => ["number", "The mouse button."]
      ],
      "return" => "boolean",
    ],
    "spry.mouse_release" => [
      "desc" => "
        Check if a mouse button was released since last frame. Mouse button 0
        is the left button, button 1 is the right button, and button 2 is the
        middle button.
      ",
      "example" => "
        if spry.mouse_release(1) then
          show_context_menu()
        end
      ",
      "args" => [
        "button" => ["number", "The mouse button."]
      ],
      "return" => "boolean",
    ],
    "spry.mouse_pos" => [
      "desc" => "Get the mouse's current position.",
      "example" => "local mx, my = spry.mouse_pos()",
      "args" => [],
      "return" => "number, number",
    ],
    "spry.mouse_delta" => [
      "desc" => "Get the mouse's movement since last frame.",
      "example" => "local dx, dy = spry.mouse_delta()",
      "args" => [],
      "return" => "number, number",
    ],
    "spry.show_mouse" => [
      "desc" => "Show/hide the mouse cursor.",
      "example" => "spry.show_mouse(false)",
      "args" => [
        "show" => ["boolean", "True if cursor should be shown. False if cursor should be hidden."],
      ],
      "return" => false,
    ],
    "spry.scroll_wheel" => [
      "desc" => "Get the scroll wheel value since last frame.",
      "example" => "local scroll_x, scroll_y = spry.scroll_wheel()",
      "args" => [],
      "return" => "number, number",
    ],
    "spry.push_matrix" => [
      "desc" => "Push a matrix onto the matrix transform stack.",
      "example" => "
        spry.push_matrix()
        spry.translate(spry.window_width() / 2, spry.window_height() / 2)
        spry.scale(self.scale, self.scale)
        spry.translate(-self.x, -self.y)
      ",
      "args" => [],
      "return" => false,
    ],
    "spry.pop_matrix" => [
      "desc" => "
        Pop a matrix from the matrix transform stack. You should have a
        matching `pop_matrix` for every `push_matrix`.
      ",
      "example" => "
        spry.push_matrix()
        spry.translate(spry.window_width() / 2, spry.window_height() / 2)
        draw_stuff()
        spry.pop_matrix()
      ",
      "args" => [],
      "return" => false,
    ],
    "spry.translate" => [
      "desc" => "Apply a 2D translation for the current matrix transform.",
      "example" => "
        spry.push_matrix()
        spry.translate(camera.x, camera.y)
      ",
      "args" => [
        "x" => ["number", "The translation in the x direction."],
        "y" => ["number", "The translation in the y direction."],
      ],
      "return" => false,
    ],
    "spry.rotate" => [
      "desc" => "Apply a rotation for the current matrix transform.",
      "example" => "
        spry.push_matrix()
        spry.rotate(camera.tilt)
      ",
      "args" => [
        "angle" => ["number", "The angle rotation in radians."],
      ],
      "return" => false,
    ],
    "spry.scale" => [
      "desc" => "Apply a 2D scale for the current matrix transform.",
      "example" => "
        spry.push_matrix()
        spry.scale(camera.zoom, camera.zoom)
      ",
      "args" => [
        "x" => ["number", "The scale in the x direction."],
        "y" => ["number", "The scale in the y direction."],
      ],
      "return" => false,
    ],
    "spry.clear_color" => [
      "desc" => "Set the background clear color. Call this before drawing the frame.",
      "example" => "spry.clear_color(255, 255, 255, 255)",
      "args" => [
        "r" => ["number", "The color's red channel, in the range [0, 255]."],
        "g" => ["number", "The color's green channel, in the range [0, 255]."],
        "b" => ["number", "The color's blue channel, in the range [0, 255]."],
        "a" => ["number", "The color's alpha channel, in the range [0, 255]."],
      ],
      "return" => false,
    ],
    "spry.push_color" => [
      "desc" => "Add a color to the color stack. The color is used as a tint when drawing things to the screen.",
      "example" => "
        if damaged then
          spry.push_color(255, 0, 0, 255)
        end
        img:draw(x, y)
      ",
      "args" => [
        "r" => ["number", "The color's red channel, in the range [0, 255]."],
        "g" => ["number", "The color's green channel, in the range [0, 255]."],
        "b" => ["number", "The color's blue channel, in the range [0, 255]."],
        "a" => ["number", "The color's alpha channel, in the range [0, 255]."],
      ],
      "return" => false,
    ],
    "spry.pop_color" => [
      "desc" => "
        Remove a color from the color stack. You should have a matching
        `pop_color` for every `push_color`.
      ",
      "example" => "
        spry.push_color(255, 0, 0, 255)
        img:draw(x, y)
        spry.pop_color()
      ",
      "args" => [],
      "return" => false,
    ],
    "spry.draw_filled_rect" => [
      "desc" => "Draw a solid filled rectangle.",
      "example" => "spry.draw_filled_rect(self.x, self.y, w, h)",
      "args" => $rect_description,
      "return" => false,
    ],
    "spry.draw_line_rect" => [
      "desc" => "Draw a rectangle outline.",
      "example" => "spry.draw_line_rect(self.x, self.y, w, h)",
      "args" => $rect_description,
      "return" => false,
    ],
    "spry.draw_line_circle" => [
      "desc" => "Draw a circle outline.",
      "example" => "spry.draw_line_circle(self.x, self.y, radius)",
      "args" => [
        "x" => ["number", "The x position to draw at."],
        "y" => ["number", "The y position to draw at."],
        "radius" => ["number", "The radius of the circle."],
      ],
      "return" => false,
    ],
  ],
  "Image" => [
    "spry.image_load" => [
      "desc" => "Create an image object from an image file.",
      "example" => "local tree_img = spry.image_load 'tree.png'",
      "args" => [
        "file" => ["string", "The image file to open."],
      ],
      "return" => "Image",
    ],
    "Image:draw" => [
      "desc" => "Draw an image onto the screen.",
      "example" => "
        -- draw image at (100, 100)
        img:draw(100, 100)

        -- draw image rotated, origin center, flipped vertically
        local ox = img:width() / 2
        local oy = img:height() / 2
        img:draw(x, y, angle, 1, -1, ox, oy)
      ",
      "args" => array_merge($draw_description, [
        "u0" => ["number", "The top-left x texture coordinate in the range [0, 1].", 0],
        "v0" => ["number", "The top-left y texture coordinate in the range [0, 1].", 0],
        "u1" => ["number", "The bottom-right x texture coordinate.", 1],
        "v1" => ["number", "The bottom-right y texture coordinate.", 1],
      ]),
      "return" => false,
    ],
    "Image:width" => [
      "desc" => "Get the width of an image.",
      "example" => "local w = img:width()",
      "args" => [],
      "return" => "number",
    ],
    "Image:height" => [
      "desc" => "Get the height of an image.",
      "example" => "local h = img:height()",
      "args" => [],
      "return" => "number",
    ],
  ],
  "Font" => [
    "spry.font_load" => [
      "desc" => "Create a font object from a `.ttf` file.",
      "example" => "local font = spry.font_load 'roboto.ttf'",
      "args" => [
        "file" => ["string", "The font file to open."],
      ],
      "return" => "Font",
    ],
    "spry.default_font" => [
      "desc" => "Get the application's default font object.",
      "example" => "local font = spry.default_font()",
      "args" => [],
      "return" => "Font",
    ],
    "Font:width" => [
      "desc" => "Get the width of the given text, drawn at a given size.",
      "example" => "local w = font:width('Hello, World!', 32)",
      "args" => [
        "text" => ["string", "The text to calucate the width with."],
        "size" => ["number", "The font size to use."],
      ],
      "return" => "number",
    ],
    "Font:draw" => [
      "desc" => "Draw text onto the screen.",
      "example" => "font:draw('Hello, World!', 100, 100, 30)",
      "args" => [
        "text" => ["string", "The text to draw."],
        "x" => ["number", "The x position to draw at.", 0],
        "y" => ["number", "The y position to draw at.", 0],
        "size" => ["number", "The size of the text.", 12],
      ],
      "return" => false,
    ],
  ],
  "Audio" => [
    "spry.set_master_volume" => [
      "desc" => "Set the master volume for the program.",
      "example" => "
        if muted then
          spry.set_master_volume(0)
        end
      ",
      "args" => [
        "volume" => ["number", "The volume in the range [0, 1]."],
      ],
      "return" => false,
    ],
    "spry.audio_load" => [
      "desc" => "Create an audio object from an `.ogg` file.",
      "example" => "local click_sound = spry.audio_load 'click.ogg'",
      "args" => [
        "file" => ["string", "The audio file to open."],
      ],
      "return" => "Audio",
    ],
    "Audio:destroy" => [
      "desc" => "Immediately stop playing a sound.",
      "example" => "click_sound:destroy()",
      "args" => [],
      "return" => false,
    ],
    "Audio:play" => [
      "desc" => "Play a sound.",
      "example" => "click_sound:play()",
      "args" => [
        "volume" => ["number", "The playback volume of the sound.", 1],
        "loop" => ["boolean", "If true, loop the playback of the sound, usually for background music.", "false"],
      ],
      "return" => false,
    ],
  ],
  "Sprite" => [
    "spry.sprite_load" => [
      "desc" => "
        Create a sprite renderer from an Aseprite file. Sprites are cached, so
        creating a sprite renderer from a file that has already been loaded
        is cheap.
      ",
      "example" => "
        function Player:new()
          self.sprite = spry.sprite_load 'player.ase'
        end
      ",
      "args" => [
        "file" => ["string", "The Aseprite file to open."],
      ],
      "return" => "SpriteRenderer",
    ],
    "SpriteRenderer:play" => [
      "desc" => "Play an animation loop with the given tag.",
      "example" => "
        if spry.key_down 'w' then
          spr:play 'walk_up'
        end
      ",
      "args" => [
        "tag" => ["string", "The tag name."]
      ],
      "return" => false,
    ],
    "SpriteRenderer:update" => [
      "desc" => "Call this method every frame to update a sprite renderer's animation state.",
      "example" => "spr:update(dt)",
      "args" => [
        "dt" => ["number", "Delta time."]
      ],
      "return" => false,
    ],
    "SpriteRenderer:draw" => [
      "desc" => "Draw a sprite on the screen.",
      "example" => "spr:draw(x, y)",
      "args" => $draw_description,
      "return" => false,
    ],
    "SpriteRenderer:width" => [
      "desc" => "Get the width of a sprite in pixels.",
      "example" => "local w = spr:width()",
      "args" => [],
      "return" => "number",
    ],
    "SpriteRenderer:height" => [
      "desc" => "Get the height of a sprite in pixels.",
      "example" => "local h = spr:height()",
      "args" => [],
      "return" => "number",
    ],
    "SpriteRenderer:set_frame" => [
      "desc" => "Set the current frame index for a sprite renderer.",
      "example" => "
        if spry.mouse_click(0) then
          sprite:set_frame(0)
        end
      ",
      "args" => [
        "frame" => ["number", "The frame index to set."],
      ],
      "return" => false,
    ],
    "SpriteRenderer:total_frames" => [
      "desc" => "Get the total number of frames of a sprite.",
      "example" => "local frames = sprite:total_frames()",
      "args" => [],
      "return" => "number",
    ],
  ],
  "Texture Atlas" => [
    "spry.atlas_load" => [
      "desc" => "
        Create an atlas object from an `.rtpa` file, generated from
        [rTexPacker](https://raylibtech.itch.io/rtexpacker). The atlas image
        (.png) is expected to be in the same directory as the `.rtpa` file.
      ",
      "example" => "local atlas = spry.atlas_load 'atlas.rtpa'",
      "args" => [
        "file" => ["string", "The .rtpa file to open."],
      ],
      "return" => "Atlas",
    ],
    "Atlas:get_image" => [
      "desc" => "
        Get an atlas image object with the given name. Atlas image objects
        store texture coordinates to draw a sub-section of a texture atlas.
      ",
      "example" => "
        local ship = atlas:get_image 'ship_01'
        ship:draw(...)
      ",
      "args" => [
        "name" => ["string", "The image name to get."],
      ],
      "return" => "AtlasImage",
    ],
    "AtlasImage:draw" => [
      "desc" => "Draw an atlas image to the screen.",
      "example" => "
        local ship = atlas:get_image 'ship_01'
        ship:draw(x, y, angle, 1, 1, ship:width() / 2, ship:height() / 2)
      ",
      "args" => $draw_description,
      "return" => false,
    ],
    "AtlasImage:width" => [
      "desc" => "Get the width of an atlas image.",
      "example" => "local offset_x = ship:width()",
      "args" => [],
      "return" => "number",
    ],
    "AtlasImage:height" => [
      "desc" => "Get the height of an atlas image.",
      "example" => "local offset_y = ship:height()",
      "args" => [],
      "return" => "number",
    ],
  ],
  "Tilemap" => [
    "spry.tilemap_load" => [
      "desc" => "
        Create a tilemap object from a LDtk file. [LDtk](https://ldtk.io/) is
        a 2D level editor.
      ",
      "example" => "local tilemap = spry.tilemap_load 'world.ldtk'",
      "args" => [
        "file" => ["string", "The tilemap file to open."],
      ],
      "return" => "Tilemap",
    ],
    "Tilemap:draw" => [
      "desc" => "Draw an entire tilemap, including all of the map's levels and layers.",
      "example" => "
        camera:begin_draw()
        tilemap:draw()
        camera:end_draw()
      ",
      "args" => [],
      "return" => false,
    ],
    "Tilemap:entities" => [
      "desc" => "Returns a list of entities for a tilemap.",
      "example" => "
        for _, v in ipairs(tilemap:entities()) do
          local Entity = lookup_entity_by_id(v.id)
          if Entity ~= nil then
            local obj = world:add(Entity(v.x, v.y))
            if v.id == 'Player' then
              player = obj
            end
          else
            print('no ' .. v.id .. ' entity exists')
          end
        end
      ",
      "args" => [],
      "return" => "table",
    ],
    "Tilemap:make_collision" => [
      "desc" => "
        Create Box2D fixtures for a tilemap. Mark certain tiles for collision
        by providing an array of IntGrid values.
      ",
      "example" => "
        b2 = spry.b2_world { gx = 0, gy = 0, meter = 16 }
        tilemap = spry.tilemap_load 'map.ldtk'
        tilemap:make_collision(b2, 16, 'Collision', { 1, 2 })
      ",
      "args" => [
        "world" => ["b2World", "The Box2D physics world."],
        "layer" => ["string", "The name of the IntGrid collision layer."],
        "walls" => ["table", "An array of numbers used for collision."],
      ],
      "return" => false,
    ],
    "Tilemap:draw_fixtures" => [
      "desc" => "
        Draw all box fixtures for a given tilemap layer.
      ",
      "example" => "
        tilemap:draw_fixtures(b2, 'Collision')
      ",
      "args" => [
        "world" => ["b2World", "The Box2D physics world."],
        "layer" => ["string", "The name of the IntGrid collision layer."],
      ],
      "return" => false,
    ],
  ],
  "Box2D World" => [
    "spry.b2_world" => [
      "desc" => "Create a new Box2D World.",
      "example" => "local b2_world = spry.b2_world { gx = 0, gy = 9.81, meter = 80 }",
      "args" => [
        "t" => ["table", "World options."],
        " .gx" => ["number", "The world's x component for gravity."],
        " .gy" => ["number", "The world's y component for gravity."],
        " .meter" => ["number", "The number of pixels for one meter."],
      ],
      "return" => "b2World",
    ],
    "b2World:step" => [
      "desc" => "Call this every frame to set the physics world in motion.",
      "example" => "
        function spry.frame(dt)
          -- update
          b2_world:step(dt)
          -- draw
          body:draw_fixtures()
        end
      ",
      "args" => [
        "dt" => ["number", "Delta time."],
        "vel_iters" => ["number", "Number of iterations in the constraint solver's velocity phase.", 6],
        "pos_iters" => ["number", "Number of iterations in the constraint solver's position phase.", 2],
      ],
      "return" => "b2World",
    ],
    "b2World:make_static_body" => [
      "desc" => "Create a static physics body.",
      "example" => "b2_world:make_static_body { x = 300, y = 400 }",
      "args" => $b2body_def,
      "return" => "b2Body",
    ],
    "b2World:make_kinematic_body" => [
      "desc" => "Create a kinematic physics body.",
      "example" => "b2_world:make_kinematic_body { x = 300, y = 400, vx = 10 }",
      "args" => $b2body_def,
      "return" => "b2Body",
    ],
    "b2World:make_dynamic_body" => [
      "desc" => "Create a dynamic physics body.",
      "example" => "b2_world:make_dynamic_body { x = 300, y = 400 }",
      "args" => $b2body_def,
      "return" => "b2Body",
    ],
    "b2World:begin_contact" => [
      "desc" => "Run a given callback function when two fixtures touch each other.",
      "example" => "
        b2_world:begin_contact(function(a, b)
          local sensor
          local other
          if a:udata() == 'sensor' then
            sensor, other = a, b
          elseif b:udata() == 'sensor' then
            sensor, other = b, a
          end

          if sensor ~= nil then
            sensor_box.count = sensor_box.count + 1
            get_actor(other):handle_sensor()
          end
        end)
      ",
      "args" => [
        "fn" => ["function", "The callback function to call on contact. Takes two arguments, both of which are fixtures."],
      ],
      "return" => false,
    ],
    "b2World:end_contact" => [
      "desc" => "Run a given callback function when two fixtures stop touching each other.",
      "example" => "
        b2_world:end_contact(function(a, b)
          local sensor
          if a:udata() == 'sensor' then
            sensor = a
          elseif b:udata() == 'sensor' then
            sensor = b
          end

          if sensor ~= nil then
            sensor_box.count = sensor_box.count - 1
          end
        end)
      ",
      "args" => [
        "fn" => ["function", "The callback function to call after contact. Takes two arguments, both of which are fixtures."],
      ],
      "return" => false,
    ],
  ],
  "Box2D Body" => [
    "b2Body:destroy" => [
      "desc" => "Immediately destroy a physics body.",
      "example" => "
        function Box:on_death()
          self.body:destroy()
        end
      ",
      "args" => [],
      "return" => false,
    ],
    "b2Body:make_box_fixture" => [
      "desc" => "Attach a box shaped fixture to a body.",
      "example" => "ground.body:make_box_fixture { w = 100, h = 50, friction = 1 }",
      "args" => array_merge([
        "t" => ["table", "Fixture definition."],
        " .x" => ["number", "The box fixture's x offset from the center of the body.", 0],
        " .y" => ["number", "The box fixture's y offset from the center of the body.", 0],
        " .w" => ["number", "The box fixture's width."],
        " .h" => ["number", "The box fixture's height."],
        " .angle" => ["number", "The box fixture's angle in radians.", 0],
      ], $b2fixture_def),
      "return" => "b2Fixture",
    ],
    "b2Body:make_circle_fixture" => [
      "desc" => "Attach a circle shaped fixture to a body.",
      "example" => "body:make_circle_fixture { radius = 3, density = 1, friction = 0.3 }",
      "args" => array_merge([
        "t" => ["table", "Fixture definition."],
        " .x" => ["number", "The box fixture's x offset from the center of the body.", 0],
        " .y" => ["number", "The box fixture's y offset from the center of the body.", 0],
        " .radius" => ["number", "The circle fixture's radius."],
      ], $b2fixture_def),
      "return" => "b2Fixture",
    ],
    "b2Body:position" => [
      "desc" => "Get the position of a physics body.",
      "example" => "local x, y = body:position()",
      "args" => [],
      "return" => "number, number",
    ],
    "b2Body:velocity" => [
      "desc" => "Get the linear velocity of a physics body.",
      "example" => "local vx, vy = body:velocity()",
      "args" => [],
      "return" => "number, number",
    ],
    "b2Body:angle" => [
      "desc" => "Get the angle of a physics body in radians.",
      "example" => "local angle = body:angle()",
      "args" => [],
      "return" => "number",
    ],
    "b2Body:linear_damping" => [
      "desc" => "Get the linear damping of a physics body.",
      "example" => "local damping = body:linear_damping()",
      "args" => [],
      "return" => "number",
    ],
    "b2Body:fixed_rotation" => [
      "desc" => "See if the physics body uses fixed rotation.",
      "example" => "local fixed = body:fixed_rotation()",
      "args" => [],
      "return" => "boolean",
    ],
    "b2Body:apply_force" => [
      "desc" => "Apply the given force to the center of a body.",
      "example" => "body:apply_force(fx, fy)",
      "args" => [
        "x" => ["number", "The force x component."],
        "y" => ["number", "The force y component."],
      ],
      "return" => false,
    ],
    "b2Body:apply_impulse" => [
      "desc" => "Apply linear impulse (immediate force) to the center of a body.",
      "example" => "body:apply_impulse(fx, fy)",
      "args" => [
        "x" => ["number", "The linear impulse x component."],
        "y" => ["number", "The linear impulse y component."],
      ],
      "return" => false,
    ],
    "b2Body:set_position" => [
      "desc" => "Set the physics body's position.",
      "example" => "body:set_position(mouse_x, mouse_y)",
      "args" => [
        "x" => ["number", "The x position to move the body to."],
        "y" => ["number", "The y position to move the body to."],
      ],
      "return" => false,
    ],
    "b2Body:set_velocity" => [
      "desc" => "Set the physics body's velocity.",
      "example" => "body:set_velocity(move_x, move_y)",
      "args" => [
        "x" => ["number", "The x velocity."],
        "y" => ["number", "The y velocity."],
      ],
      "return" => false,
    ],
    "b2Body:set_angle" => [
      "desc" => "Set the physics body's angle.",
      "example" => "body:set_angle(angle)",
      "args" => [
        "angle" => ["number", "The angle to set."],
      ],
      "return" => false,
    ],
    "b2Body:set_linear_damping" => [
      "desc" => "Set the physics body's linear damping.",
      "example" => "body:set_linear_damping(30)",
      "args" => [
        "damping" => ["number", "The linear damping to set."],
      ],
      "return" => false,
    ],
    "b2Body:set_fixed_rotation" => [
      "desc" => "Fix the rotation of a physics body.",
      "example" => "body:set_fixed_rotation(true)",
      "args" => [
        "angle" => ["boolean", "If true, prevent the body from rotating."],
      ],
      "return" => false,
    ],
    "b2Body:set_transform" => [
      "desc" => "Set the physics body's position and angle.",
      "example" => "body:set_transform(x, y, angle)",
      "args" => [
        "x" => ["number", "The x position to move the body to."],
        "y" => ["number", "The y position to move the body to."],
        "angle" => ["number", "The angle to set."],
      ],
      "return" => false,
    ],
    "b2Body:draw_fixtures" => [
      "desc" => "Draws all fixtures attached to a physics body.",
      "example" => "
        if show_debug_info then
          show_fps()
          body:draw_fixtures()
        end
      ",
      "args" => [],
      "return" => false,
    ],
    "b2Body:udata" => [
      "desc" => "Get the attached user data for a physics body.",
      "example" => "print(body:udata())",
      "args" => [],
      "return" => "string | number",
    ],
  ],
  "Box2D Fixture" => [
    "b2Fixture:friction" => [
      "desc" => "Get the friction value of a fixture.",
      "example" => "
        if fixture:friction() > 0.95 then
          actor.is_sandpaper = true
        end
      ",
      "args" => [],
      "return" => "number",
    ],
    "b2Fixture:restitution" => [
      "desc" => "Get the restitution value of a fixture.",
      "example" => "
        if fixture:restitution() > 0.9 then
          actor.is_bouncy = true
        end
      ",
      "args" => [],
      "return" => "number",
    ],
    "b2Fixture:is_sensor" => [
      "desc" => "Returns true if a fixture is a sensor.",
      "example" => "
        function on_begin_contact(a, b)
          if a:is_sensor() or b:is_sensor() then
            return
          end

          handle_contact(a, b)
        end
      ",
      "args" => [],
      "return" => "boolean",
    ],
    "b2Fixture:set_friction" => [
      "desc" => "Set the fixture's friction value.",
      "example" => "
        function Ball:on_contact()
          self.fixture:set_friction(5)
        end
      ",
      "args" => [
        "friction" => ["number", "The friction value. Typically between [0, 1]."],
      ],
      "return" => false,
    ],
    "b2Fixture:set_restitution" => [
      "desc" => "Set the fixture's restitution value.",
      "example" => "
        function Ball:on_contact()
          self.fixture:set_restitution(0)
        end
      ",
      "args" => [
        "restitution" => ["number", "The restitution value. Typically between [0, 1]."],
      ],
      "return" => false,
    ],
    "b2Fixture:set_sensor" => [
      "desc" => "Set the fixture's sensor state.",
      "example" => "fixture:set_sensor(true)",
      "args" => [
        "sensor" => ["boolean", "True if the fixture should be a sensor."],
      ],
      "return" => false,
    ],
    "b2Fixture:body" => [
      "desc" => "Get the physics body of a fixture.",
      "example" => "local str = fixture:body():udata()",
      "args" => [],
      "return" => "b2Body",
    ],
    "b2Fixture:udata" => [
      "desc" => "Get the attached user data for a fixture.",
      "example" => "print(fixture:udata())",
      "args" => [],
      "return" => "string | number",
    ],
  ],
  "2D Vector" => [
    "vec2" => [
      "desc" => "Create a new 2D vector.",
      "example" => "
        local a = vec2(self.x, self.y)
        local b = vec2(other.x, other.y)
        local c = a + b
        print(c.x, c.y)
      ",
      "args" => [
        "x" => ["number", "The vector's x component."],
        "y" => ["number", "The vector's y component."],
      ],
      "return" => "vec2",
    ],
    "vec2:normalize" => [
      "desc" => "Get a vector with length 1, or vec2(0, 0) if this vector is also vec2(0, 0).",
      "example" => "local move_dir = input_axes:normalize()",
      "args" => [],
      "return" => "vec2",
    ],
    "vec2:distance" => [
      "desc" => "Return the distance between two vectors.",
      "example" => "local dist = pos:distance(enemy)",
      "args" => [
        "rhs" => ["vec2", "Another vector."],
      ],
      "return" => "number",
    ],
    "vec2:direction" => [
      "desc" => "Return the direction from this vector to `rhs` in radians.",
      "example" => "local angle = pos:angle(enemy)",
      "args" => [
        "rhs" => ["vec2", "Another vector."],
      ],
      "return" => "number",
    ],
    "vec2:lerp" => [
      "desc" => "Linearly interpolate between this vector and rhs. Produces a new vector.",
      "example" => "pos = pos:lerp(other, dt)",
      "args" => [
        "rhs" => ["vec2", "Another vector."],
        "dt" => ["number", "Delta time."],
      ],
      "return" => "vec2",
    ],
    "vec2:dot" => [
      "desc" => "The dot product of this vector and rhs.",
      "example" => "local dp = vel:dot(normal)",
      "args" => [
        "rhs" => ["vec2", "Another vector."],
      ],
      "return" => "number",
    ],
    "vec2:unpack" => [
      "desc" => "Returns the x and y components of this vector.",
      "example" => "local x, y = vel:unpack()",
      "args" => [],
      "return" => "number, number",
    ],
    "vec2.__add" => [
      "desc" => "Add two vectors.",
      "example" => "local v3 = v1 + v2",
      "args" => [
        "lhs" => ["vec2", "Left hand side vector."],
        "rhs" => ["vec2", "Right hand side vector."],
      ],
      "return" => "vec2",
    ],
    "vec2.__sub" => [
      "desc" => "Subtract two vectors.",
      "example" => "local v3 = v1 - v2",
      "args" => [
        "lhs" => ["vec2", "Left hand side vector."],
        "rhs" => ["vec2", "Right hand side vector."],
      ],
      "return" => "vec2",
    ],
    "vec2.__mul" => [
      "desc" => "Multiply two vectors.",
      "example" => "local v3 = v1 * v2",
      "args" => [
        "lhs" => ["vec2", "Left hand side vector."],
        "rhs" => ["vec2", "Right hand side vector."],
      ],
      "return" => "vec2",
    ],
    "vec2.__div" => [
      "desc" => "Divide two vectors.",
      "example" => "local v3 = v1 / v2",
      "args" => [
        "lhs" => ["vec2", "Left hand side vector."],
        "rhs" => ["vec2", "Right hand side vector."],
      ],
      "return" => "vec2",
    ],
    "vec2:__len" => [
      "desc" => "Get the length of a vector.",
      "example" => "local length = #vel",
      "args" => [],
      "return" => "number",
    ],
    "vec2.__eq" => [
      "desc" => "Check if two vectors are the same.",
      "example" => "print(v1 == v2)",
      "args" => [
        "lhs" => ["vec2", "Left hand side vector."],
        "rhs" => ["vec2", "Right hand side vector."],
      ],
      "return" => "boolean",
    ],
  ],
  "World" => [
    "World" => [
      "desc" => "
        Create a new world object, which manages object/actor creation,
        updates, and destruction. An actor is a class with a constructor, an
        `update` method and a `draw` method.
      ",
      "example" => "
        function spry.start()
          world = World()

          interval(2, function()
            world:add(Enemy())
          end)

          player = world:add(Player())
        end

        function spry.frame(dt)
          world:update(dt)
          world:draw()
        end

        -- example Player class

        class 'Player'

        function Player:new(x, y)
          self.x, self.y = x, y
        end

        function Player:on_create()
          -- optional. called after object is inserted into the world.
          -- use this to get the id before the first object update.
        end

        function Player:on_death()
          -- optional. called right before the object is removed.
          -- use this to clean up things like physics bodies.
        end

        function Player:update(dt)
          -- required. called during world update phase.
        end

        function Player:draw()
          -- required. called during world draw phase.
        end
      ",
      "args" => [],
      "return" => "World",
    ],
    "World:add" => [
      "desc" => "
        Add a new actor to the world, returns the actor passed in. When
        adding an actor, they are assigned an unique id.
      ",
      "example" => "
        local player = world:add(Player())
        print(player.id)
      ",
      "args" => [
        "obj" => ["table", "The actor to add. It should have an update and draw method."],
      ],
      "return" => "table",
    ],
    "World:kill" => [
      "desc" => "Remove an actor from the world given an actor's id, or the actor itself.",
      "example" => "
        if bullet_hit_enemy then
          world:kill(enemy)
          world:kill(self)
        end
      ",
      "args" => [
        "obj" => ["number | table", "The actor to remove."],
      ],
      "return" => false,
    ],
    "World:query_id" => [
      "desc" => "Find an actor with the given id.",
      "example" => "local enemy = world:query_id(self.enemy_id)",
      "args" => [
        "id" => ["number", "The actor id."],
      ],
      "return" => "table",
    ],
    "World:query_mt" => [
      "desc" => "Find actors with the given metatable. Returns a table of entities.",
      "example" => "
        local bullets = world:query_mt(Bullet)
        for id, bullet in pairs(bullets) do
          bullet_stuff(bullet)
        end
      ",
      "args" => [
        "mt" => ["table", "The metatable to search with."],
      ],
      "return" => "table",
    ],
    "World:update" => [
      "desc" => "Call this method every frame to update the world's state.",
      "example" => "world:update(dt)",
      "args" => [
        "dt" => ["number", "Delta time."]
      ],
      "return" => false,
    ],
    "World:draw" => [
      "desc" => "Call this method every frame to draw all entities in the world.",
      "example" => "world:draw()",
      "args" => [],
      "return" => false,
    ],
  ],
  "Entity Component System" => [
    "ECS" => [
      "desc" => "Create a new ECS object. Entities are created and removed after [`ECS:update()`](#ecs:update).",
      "example" => "
        function spry.start()
          ecs = ECS()

          for i = 1, 100 do
            ecs:add {
              pos = { x = 200, y = 200 },
              vel = { x = random(-100, 100), y = random(-100, 100) },
            }
          end
        end

        function spry.frame(dt)
          ecs:update()

          for id, e in ecs:select { 'pos', 'vel' } do
            e.pos.x = e.pos.x + e.vel.x * dt
            e.pos.y = e.pos.y + e.vel.y * dt

            -- adding a component
            if add_accel then
              e.accel = { x = ax, y = ay }
            end

            -- removing a component
            if remove_vel then
              e.vel = nil
            end
          end
        end
      ",
      "args" => [],
      "return" => "ECS",
    ],
    "ECS:update" => [
      "desc" => "Call this method every frame so that entities are created and removed properly.",
      "example" => "
        function spry.frame(dt)
          ecs:update()
          -- the rest of the frame
        end
      ",
      "args" => [],
      "return" => false,
    ],
    "ECS:add" => [
      "desc" => "Create a new entity. Returns the entity id and the given entity itself.",
      "example" => "
        ecs:add {
          pos = { x = spry.window_width() / 2, y = spry.window_height() / 2 },
          vel = { x = random(-100, 100), y = random(-100, 100) },
        }
      ",
      "args" => [
        "entity" => ["table", "The entity to add, where each table entry is a component."],
      ],
      "return" => "number, table",
    ],
    "ECS:kill" => [
      "desc" => "Given an entity id, remove an entity.",
      "example" => "
        if e.stats.hp == 0 then
          ecs:kill(id)
        end
      ",
      "args" => [
        "id" => ["number", "The entity id."],
      ],
      "return" => false,
    ],
    "ECS:get" => [
      "desc" => "Returns the entity with the given id, or `nil` if it doesn't exist.",
      "example" => "
        local other = ecs:get(e.owner.id)
        if other ~= nil then
          other.ammo.amount = other.ammo.amount - 1
        end
      ",
      "args" => [
        "id" => ["number", "The entity id."],
      ],
      "return" => "table",
    ],
    "ECS:query" => [
      "desc" => "
        Returns an iterator over entities that match the given query. The
        iterator returns the id and entity of each element.
      ",
      "example" => "
        for id, e in ecs:query {
          select = { 'pos', 'img', 'z_index' },
          where = function(e)
            return e.pos.x < spry.window_width() / 2
          end,
          order_by = function(lhs, rhs)
            return lhs.entity.z_index < rhs.entity.z_index
          end,
        } do
          e.pos = e.pos + e.vel * vec2(dt, dt)
          e.img:draw(e.pos.x, e.pos.y)
        end
      ",
      "args" => [
        "t" => ["table", "Query definition."],
        " .select" => ["table", "The names of each component."],
        " .where" => ["function", "A function used to filter out entities.", "nil"],
        " .order_by" => ["function", "A function used to sort entities.", "nil"],
      ],
      "return" => "iterator",
    ],
    "ECS:select" => [
      "desc" => "Returns an iterator over entities with the given components.",
      "example" => "
        for id, e in ecs:select { 'pos', 'vel' } do
          e.pos = e.pos + e.vel * vec2(dt, dt)
        end
      ",
      "args" => [
        "columns" => ["table", "The names of each component."],
      ],
      "return" => "iterator",
    ]
  ],
  "Spring" => [
    "Spring" => [
      "desc" => "Create a new spring object, which can simulate bouncy behaviour.",
      "example" => "self.spring = Spring()",
      "args" => [
        "k" => ["number", "The spring's stiffness.", 400],
        "d" => ["number", "The spring's damping.", 28],
      ],
      "return" => "Spring",
    ],
    "Spring:update" => [
      "desc" => "Call this method every frame to update a spring's state.",
      "example" => "spring:update(dt)",
      "args" => [
        "dt" => ["number", "Delta time."]
      ],
      "return" => false,
    ],
    "Spring:pull" => [
      "desc" => "Pull on a string with given force.",
      "example" => "
        if damage_hit then
          self.spring:pull(5)
        end
      ",
      "args" => [
        "f" => ["number", "The force to pull with."]
      ],
      "return" => false,
    ],
  ],
  "Timer" => [
    "interval" => [
      "desc" => "Repeat a callback function for a given amount of seconds.",
      "example" => "
        interval(5, function()
          spawn_enemy()
        end)
      ",
      "args" => [
        "sec" => ["number", "Number of seconds."],
        "action" => ["function", "The callback function."],
      ],
      "return" => "number",
    ],
    "timeout" => [
      "desc" => "After a given amount of seconds, execute a callback once.",
      "example" => "
        timeout(3, function()
          after_countdown()
        end)
      ",
      "args" => [
        "sec" => ["number", "Number of seconds."],
        "action" => ["function", "The callback function."],
      ],
      "return" => "number",
    ],
    "stop_interval" => [
      "desc" => "Stop executing the callback from `interval`.",
      "example" => "
        local timer = interval(3, fn)
        stop_interval(timer)
      ",
      "args" => [
        "id" => ["number", "The return value from calling the interval function."],
      ],
      "return" => false,
    ],
    "stop_timeout" => [
      "desc" => "Cancel a `timeout`.",
      "example" => "
        local timer = timeout(3, fn)
        stop_timeout(timer)
      ",
      "args" => [
        "id" => ["number", "The return value from calling the timeout function."],
      ],
      "return" => false,
    ],
  ],
  "Utility Functions" => [
    "require" => [
      "desc" => "
        Include a Lua file relative to the root of the project directory. The
        format is in a similar style as the `require` function in vanilla
        Lua. Path separators are `.` instead of `/`, and the `.lua` extension
        is excluded.
      ",
      "example" => "
        function spry.start()
          lume = require 'deps.lume'
        end
      ",
      "args" => [
        "file" => ["string", "The file to include."],
      ],
      "return" => "any",
    ],
    "class" => [
      "desc" => "Create a new class.",
      "example" => "
        class 'Player'

        function Player:new(x, y)
          self.x = x
          self.y = y
        end

        function Player:update(dt)
          self.x = self.x + dt * 10
        end

        function Player:draw()
          img:draw(self.x, self.y)
        end
      ",
      "args" => [
        "name" => ["string", "The name of the class."],
      ],
      "return" => false,
    ],
    "stringify" => [
      "desc" => "Get a string representation of a value, table fields included (unlike `tostring`).",
      "example" => "print(stringify(value))",
      "args" => [
        "value" => ["any", "A value to convert to a string."],
      ],
      "return" => "string",
    ],
    "clamp" => [
      "desc" => "Clamp a number to the range [min, max].",
      "example" => "local speed = clamp(speed, 1, 10)",
      "args" => [
        "n" => ["number", "The number to clamp."],
        "min" => ["number", "The min number in the range."],
        "max" => ["number", "The max number in the range."],
      ],
      "return" => "number",
    ],
    "sign" => [
      "desc" => "Get the sign of a number. Returns -1, 1, or 0.",
      "example" => "local dir_x = sign(dst_x - src_x)",
      "args" => [
        "x" => ["number", "The number to get the sign of."],
      ],
      "return" => "number",
    ],
    "lerp" => [
      "desc" => "Linearly interpolate between two numbers.",
      "example" => "x = lerp(x, other_x, dt)",
      "args" => [
        "src" => ["number", "The value at t = 0."],
        "dest" => ["number", "The value at t = 1."],
        "t" => ["number", "The interpolation amount."],
      ],
      "return" => "number",
    ],
    "direction" => [
      "desc" => "Direction from (x0, y0) to (x1, y1) in radians.",
      "example" => "local angle = direction(self.x, self.y, x, y)",
      "args" => [
        "x0" => ["number", "The source x component."],
        "y0" => ["number", "The source y component."],
        "x1" => ["number", "The destination x component."],
        "y1" => ["number", "The destination y component."],
      ],
      "return" => "number",
    ],
    "heading" => [
      "desc" => "Return x and y coordinates for a heading vector described by an angle and magnitude.",
      "example" => "local move_x, move_y = heading(angle_to_mouse, speed)",
      "args" => [
        "angle" => ["number", "The angle of the vector."],
        "mag" => ["number", "The magnitude of the vector."],
      ],
      "return" => "number, number",
    ],
    "delta_angle" => [
      "desc" => "Get angle difference between two angles in the range [-pi, pi).",
      "example" => "local angle = delta_angle(self.angle, angle_to_mouse)",
      "args" => [
        "src" => ["number", "The source angle in radians."],
        "dst" => ["number", "The destination angle in radians."],
      ],
      "return" => "number",
    ],
    "distance" => [
      "desc" => "Get distance between two points.",
      "example" => "local length = distance(self.x, self.y, x, y)",
      "args" => [
        "x0" => ["number", "The source x component."],
        "y0" => ["number", "The source y component."],
        "x1" => ["number", "The destination x component."],
        "y1" => ["number", "The destination y component."],
      ],
      "return" => "number",
    ],
    "normalize" => [
      "desc" => "Get a point representing a 2D vector with length of 1, or (0, 0) if length of (x, y) is 0.",
      "example" => "x, y = normalize(x, y)",
      "args" => [
        "x" => ["number", "The point's x component."],
        "y" => ["number", "The point's y component."],
      ],
      "return" => "number, number",
    ],
    "dot" => [
      "desc" => "Get the dot product of two points.",
      "example" => "local dp = dot(vel_x, vel_y, other_x, other_y)",
      "args" => [
        "x0" => ["number", "The x component of a point."],
        "y0" => ["number", "The y component of a point."],
        "x1" => ["number", "The x component of another point."],
        "y1" => ["number", "The y component of another point."],
      ],
      "return" => "number",
    ],
    "random" => [
      "desc" => "
        Get a random number in the range [min, max). Equivalent to
        `math.random()` if min = 0 and max = 1.
      ",
      "example" => "local r = random(1, 100)",
      "args" => [
        "min" => ["number", "The smallest number than can be generated."],
        "max" => ["number", "The largest number than can be generated."],
      ],
      "return" => "number",
    ],
    "clone" => [
      "desc" => "Creates a new shallow copy of a table.",
      "example" => "local bullets_copy = clone(bullets)",
      "args" => [
        "t" => ["table", "The table to clone."],
      ],
      "return" => "table",
    ],
    "push" => [
      "desc" => "Add a value to the end of a table.",
      "example" => "push(bullets, bullet)",
      "args" => [
        "arr" => ["table", "The table to push a value to."],
        "x" => ["any", "The value to push."],
      ],
      "return" => false,
    ],
    "map" => [
      "desc" => "Create a new table with the result of calling a function on each item.",
      "example" => "
        local nums = {1, 2, 3, 4, 5}
        local doubled = map(nums, function(n) return n * 2 end)
      ",
      "args" => [
        "arr" => ["table", "The array to apply a callback function to."],
        "fn" => ["any", "The callback function."],
      ],
      "return" => "table",
    ],
    "filter" => [
      "desc" => "Create a new table with items that pass the given function.",
      "example" => "
        local nums = {1, 2, 3, 4, 5}
        local odd_nums = filter(nums, function(n) return n % 2 == 1 end)
      ",
      "args" => [
        "arr" => ["table", "The array to filter items with."],
        "fn" => ["any", "The predicate function."],
      ],
      "return" => "table",
    ],
    "zip" => [
      "desc" => "
        Return a table where each element is a pair of items from two other
        tables. If table lengths differ, the shortest table's length is
        used.
      ",
      "example" => "
        local nums = zip({1, 2, 3}, {'one', 'two', 'three'})
        -- nums == {{1, 'one'}, {2, 'two'}, {3, 'three'}}
      ",
      "args" => [
        "lhs" => ["table", "The first array."],
        "rhs" => ["table", "The second array."],
      ],
      "return" => "table",
    ],
    "choose" => [
      "desc" => "Choose a random value in an array.",
      "example" => "card = choose(cards)",
      "args" => [
        "arr" => ["table", "The array to pick from."],
      ],
      "return" => "any",
    ],
    "sortpairs" => [
      "desc" => "Returns an iterator over a table sorted by key.",
      "example" => "
        local t = {
          c = 'third',
          b = 'second',
          a = 'first',
        }

        for k, v in sortpairs(t) do
          print(v)
        end

        -- first
        -- second
        -- third
      ",
      "args" => [
        "t" => ["table", "The table to iterate."],
      ],
      "return" => "iterator",
    ],
    "create_thread" => [
      "desc" => "Creates a coroutine.",
      "example" => "local update_thread = create_thread(co_update)",
      "args" => [
        "fn" => ["function", "The function to create the coroutine with."],
      ],
      "return" => "thread",
    ],
    "resume" => [
      "desc" => "Runs a coroutine, raising an error if the coroutine ran with an error.",
      "example" => "resume(thread, self, dt)",
      "args" => [
        "co" => ["thread", "The coroutine to resume."],
        "..." => [false, "Arguments to pass to the coroutine."],
      ],
      "return" => false,
    ],
    "yield" => [
      "desc" => "
        Suspend a coroutine. Similar to `coroutine.yield(...)` but it ignores the
        first return value. This is useful when the first argument passed to
        the coroutine doesn't change (for example, when passing `self`).
      ",
      "example" => "
        function Enemy:new()
          self.thread = create_thread(self.wander)
        end

        function Enemy:wander(dt)
          while true do
            -- stuff
            dt = yield()
          end
        end

        function Enemy:update(dt)
          resume(self.thread, self, dt)
        end
      ",
      "args" => [
        "..." => [false, "Arguments to pass to whoever resumed the coroutine."],
      ],
      "return" => "any",
    ],
    "sleep" => [
      "desc" => "Pause a coroutine for given number of seconds. Returns the next delta time.",
      "example" => "
        function Camera:begin_cutscene()
          -- call cutscene_thread
          resume(self.thread, self)
        end

        function Camera:cutscene_thread()
          self:move_to(door.x, door.y)
          door:open()
          sleep(3)
          self:move_to(player.x, player.y)
        end
      ",
      "args" => [
        "secs" => ["number", "The number of seconds to sleep for."],
      ],
      "return" => "number",
    ],
  ],
];

?>
<div class="mw8 center">
  <div
    class="dn db-l fixed bottom-0 pl1 pr2 pb2 overflow-y-scroll overflow-x-hidden"
    style="width: 300px; top: <?= data()->nav_height ?>"
  >
    <form
      onsubmit="event.preventDefault()"
      class="top-0 pt3 pb4 flex items-center bg-fade-down"
      style="position: sticky"
    >
      <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" style="width: 20px; height: 20px; margin-bottom: 1px" class="gray absolute ml2">
        <path fill-rule="evenodd" d="M9 3.5a5.5 5.5 0 100 11 5.5 5.5 0 000-11zM2 9a7 7 0 1112.452 4.391l3.328 3.329a.75.75 0 11-1.06 1.06l-3.329-3.328A7 7 0 012 9z" clip-rule="evenodd" />
      </svg>
      <input
        id="search"
        autocomplete="off"
        class="input-reset shadow-sm ba b--black-20 dm-b--white-20 placeholder black dm-white br2 pv2 pl4 pr2 w-100 bg-white dm-bg-black"
        type="text"
        placeholder="Search"
        style="outline-offset: 2px"
      />
    </form>
    <ul id="function-list" class="list pl1 mt0" style="margin-top: -1rem">
      <?php foreach ($api_reference as $header => $section): ?>
        <li>
          <span class="dib fw6 mt3 mb2"><?= $header ?></span>
          <ul class="list pl0 mt0">
            <?php foreach ($section as $name => $func): ?>
              <li class=" pv1" data-key="<?= strtolower($name) ?>">
                <a href="#<?= strtolower($name) ?>" class="dark-gray dm-silver link underline-hover lh-solid dib">
                  <code><?= $name ?></code>
                </a>
              </li>
            <?php endforeach ?>
          </ul>
        </li>
      <?php endforeach ?>
    </ul>
  </div>
  <div class="pl3-l pt3 ml-300px-l">
    <?php foreach ($api_reference as $header => $section): ?>
      <?php foreach ($section as $name => $func): ?>
        <div class="br3 ba bg-white dm-bg-black-20 b--black-10 dm-b--white-10 pa3 mb3 shadow-sm">
          <span id="<?= strtolower($name) ?>" style="position: relative; top: -5rem"></span>
          <p class="mv0 f6 fw6 gray">
            <?= $header ?>
          </p>
          <h2 class="mb2 f4">
            <a href="#<?= strtolower($name) ?>" class="black dm-white link underline-hover break-words" style="letter-spacing: -1px">
              <code>
                <?php
                $args = array_keys($func["args"]);
                $args = array_filter($args, fn($a) => !str_starts_with($a, " "));
                echo $name . "(" . implode(", ", $args) . ")";
                ?>
              </code>
            </a>
          </h2>
          <div class="lh-copy prose">
            <?= Parsedown::instance()->text(multiline_trim($func["desc"])); ?>
          </div>
          <?php if (count($func["args"]) > 0): ?>
            <?php
            $has_default = false;
            foreach ($func["args"] as $arr) {
              if (count($arr) == 3) {
                $has_default = true;
                break;
              }
            }
            ?>
            <h4 class="mid-gray dm-moon-gray mt3 mb2">Arguments</h4>
            <div class="ba b--black-10 dm-b--white-10 dib bg-off-white dm-bg-black-10 br3 overflow-hidden mw-100">
              <div class="overflow-x-auto">
                <table class="collapse nowrap">
                  <tbody>
                    <tr class="bg-black-10 dm-bg-white-10 f6 black-70 dm-white-80">
                      <th class="tl pv2 ph3 fw6">Name</th>
                      <th class="tl pv2 ph3 fw6">Type</th>
                      <?php if ($has_default): ?>
                        <th class="tl pv2 ph3 fw6">Default</th>
                      <?php endif ?>
                      <th class="tl pv2 ph3 fw6">Description</th>
                    </tr>
                    <?php foreach ($func["args"] as $arg_name => $arg): ?>
                      <?php
                      $type = $arg[0];
                      $desc = $arg[1];
                      $default = $arg[2] ?? false;
                      ?>
                      <tr>
                        <td class="pv2 ph3 pre overflow-hidden"><code><?= $arg_name ?></code></td>
                        <td class="pv2 ph3">
                          <?php if ($type): ?>
                            <code><?= $type ?></code>
                          <?php else: ?>
                            <span class="i gray">N/A</span>
                          <?php endif ?>
                        </td>
                        <?php if ($has_default): ?>
                          <td class="pv2 ph3">
                            <?php if ($default !== false): ?>
                              <code><?= $default ?></code>
                            <?php endif ?>
                          </td>
                        <?php endif ?>
                        <td class="pv2 ph3"><?= $desc ?></td>
                      </tr>
                    <?php endforeach ?>
                  </tbody>
                </table>
              </div>
            </div>
          <?php endif ?>

          <?php if ($func["return"]): ?>
            <p>
              <span class="mid-gray dm-moon-gray mt3 mb2 fw6">Returns</span>
              <code class="inline-code"><?= $func["return"] ?></code>.
            </p>
          <?php else: ?>
            <p class="i gray">Returns nothing.</p>
          <?php endif ?>

          <div class="prose relative">
            <span class="dib absolute top-0 left-0 pt1 pl2 br3 gray fw6 f7 ttu">
              Example
            </span>
            <pre class="mv0"><code class="language-lua" style="padding-top: 1.5rem"><?= multiline_trim($func["example"]) ?></code></pre>
          </div>
        </div>
      <?php endforeach ?>
    <?php endforeach ?>
  </div>
</div>

<script>
  const search = document.getElementById('search')

  function updateFunctionList() {
    const groups = document.querySelectorAll('#function-list > li')
    for (const group of groups) {
      const items = group.querySelectorAll("ul > li")

      let has_items = false
      for (const item of items) {
        if (item.dataset.key.includes(search.value.toLowerCase())) {
          has_items = true
          item.style.display = 'list-item'
        } else {
          item.style.display = 'none'
        }
      }

      group.style.display = has_items ? 'list-item' : 'none'
    }
  }

  search.addEventListener('input', updateFunctionList)

  document.addEventListener('keydown', e => {
    if (e.code === 'Slash' && document.activeElement !== search) {
      e.preventDefault()
      search.focus()
      search.select()
    } else if (e.code === 'Escape') {
      if (search.value.length > 0) {
        search.value = ''
        updateFunctionList()
      } else {
        search.blur()
      }
    }
  })
</script>
