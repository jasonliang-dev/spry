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

$mu_rect = [
  "rect" => ["table", "Rectangle definition."],
  " .x" => ["number", "The top left x position."],
  " .y" => ["number", "The top left y position."],
  " .w" => ["number", "The width of the rectangle."],
  " .h" => ["number", "The height of the rectangle."],
];

$mu_color = [
  "color" => ["table", "Color definition."],
  " .r" => ["number", "The color's red channel, in the range [0, 255]."],
  " .g" => ["number", "The color's green channel, in the range [0, 255]."],
  " .b" => ["number", "The color's blue channel, in the range [0, 255]."],
  " .a" => ["number", "The color's alpha channel, in the range [0, 255]."],
];

$mu_ref_desc = [
  "<a class=\"blue no-underline underline-hover\" href=\"#microui.ref\">
    mu_Ref
  </a>",
];

$api_reference = [
  "Callback Functions" => [
    "spry.arg" => [
      "desc" => "
        Define this callback function to read command line arguments before
        anything else runs.

        You should return a table with `console` as the key, and a boolean
        result as the value. This enables/disables the console output on
        Windows. If the value of `console` is false, The value of
        `win_console` in `spry.conf` is used instead.
      ",
      "example" => "
        function spry.arg(arg)
          local usage = false
          local console = false

          for _, a in ipairs(arg) do
            if a == '--help' or a == '-h' then
              usage = true
            elseif a == '--console' then
              console = true
            end
          end

          if usage then
            local str = ([[usage:
          %s [command...]
        commands:
          --help, -h  show help
          --console   use console output
        ]]):format(spry.program_path())

            print(str)
            os.exit()
          end

          return { console = console }
        end
      ",
      "args" => [
        "arg" => ["table", "Command line arguments."],
      ],
      "return" => "table",
    ],
    "spry.conf" => [
      "desc" => "Define this callback function to configure various program options.",
      "example" => "
        function spry.conf(t)
          t.window_width = 1280
          t.window_height = 720
        end
      ",
      "args" => [
        "t" => ["table", "The table to edit options with."],
        " .win_console" => ["boolean", "Windows only. Use console output.", "false"],
        " .hot_reload" => ["boolean", "Enable/disable hot reloading of scripts and assets.", "true"],
        " .startup_load_scripts" => ["boolean", "Enable/disable loading all lua scripts in the project.", "true"],
        " .fullscreen" => ["boolean", "If true, start the program in fullscreen mode.", "false"],
        " .reload_interval" => ["number", "The time in seconds to update files for hot reloading.", 0.1],
        " .swap_interval" => ["number", "Set the swap interval. Typically 1 for VSync, or 0 for no VSync.", 1],
        " .target_fps" => ["number", "Set the maximum frames to render per second. No FPS limit if target is 0.", 0],
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
        function spry.start(arg)
          if os.getenv 'LOCAL_LUA_DEBUGGER_VSCODE' == '1' then
            unsafe_require('lldebugger').start()
          end

          img = spry.image_load 'tile.png'
          font = spry.font_load 'roboto.ttf'
        end
      ",
      "args" => [
        "arg" => ["table", "Command line arguments."],
      ],
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
  "Core" => [
    "spry.version" => [
      "desc" => "Get the version of Spry.",
      "example" => "print(spry.version())",
      "args" => [],
      "return" => "string",
    ],
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
    "spry.program_path" => [
      "desc" => "Returns the path to this executable.",
      "example" => "
        if spry.platform() ~= 'html5' then
          print(spry.program_path())
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
    "spry.fullscreen" => [
      "desc" => "Returns true if program is fullscreen.",
      "example" => "local fs = spry.fullscreen()",
      "args" => [],
      "return" => "boolean",
    ],
    "spry.toggle_fullscreen" => [
      "desc" => "Toggle fullscreen.",
      "example" => "
        if spry.key_press 'f11' then
          spry.toggle_fullscreen()
        end
      ",
      "args" => [],
      "return" => false,
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
    "spry.time" => [
      "desc" => "
        Get the current time in nanoseconds. The result is only useful when
        comparing against other calls to `spry.time`.
      ",
      "example" => "local now = spry.time()",
      "args" => [],
      "return" => "number",
    ],
    "spry.difftime" => [
      "desc" => "Get the time elapsed from `t1` to `t2` in nanoseconds.",
      "example" => "
        local milliseconds = 1000000

        now = spry.time()
        contents = read_entire_file 'map.ldtk'
        print(spry.difftime(spry.time(), now) / milliseconds)
      ",
      "args" => [
        "t2" => ["number", "Time in nanoseconds, should be after t1."],
        "t1" => ["number", "Time in nanoseconds, should be before t2."],
      ],
      "return" => "number",
    ],
    "spry.json_read" => [
      "desc" => "Deserialize a JSON string into a Lua value.",
      "example" => "local data, err = spry.json_read [[{
        'x':10,
        'y':20,
        'fruits': ['apple','banana','orange']
      }]]",
      "args" => [
        "str" => ["string", "The JSON string."],
      ],
      "return" => [
        "on success" => "any",
        "on failure" => "nil, string",
      ],
    ],
    "spry.json_write" => [
      "desc" => "Serialize a Lua value into a JSON string.",
      "example" => "local data, err = spry.json_read [[{
        'x':10,
        'y':20,
        'fruits': ['apple','banana','orange']
      }]]",
      "args" => [
        "str" => ["string", "The JSON string."],
      ],
      "return" => [
        "on success" => "string",
        "on failure" => "nil, string",
      ],
    ],
  ],
  "Input" => [
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
  ],
  "Drawing" => [
    "spry.scissor_rect" => [
      "desc" => "Limit drawing within a rectangle.",
      "example" => "
        function Card:draw()
          spry.scissor_rect(self.px, self.py, self.pw, self.ph)
          font:draw(self.text, self.text_x, self.text_y, 24)
        end
      ",
      "args" => [
        "x" => ["number", "The top left x position of the rectangle.", 0],
        "y" => ["number", "The top left y position of the rectangle.", 0],
        "w" => ["number", "The width of the rectangle.", "spry.window_width()"],
        "h" => ["number", "The height of the rectangle.", "spry.window_height()"],
      ],
      "return" => false,
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
    "spry.draw_line" => [
      "desc" => "Draw a line between two points.",
      "example" => "
        local path = tilemap:astar(...)
        for i = 1, #path - 1 do
          local x0 = path[i + 0].x + off
          local y0 = path[i + 0].y + off
          local x1 = path[i + 1].x + off
          local y1 = path[i + 1].y + off

          spry.draw_line(x0, y0, x1, y1)
        end
      ",
      "args" => [
        "x0" => ["number", "The starting x position to draw at."],
        "y0" => ["number", "The starting y position to draw at."],
        "x1" => ["number", "The ending x position to draw at."],
        "y1" => ["number", "The ending y position to draw at."],
      ],
      "return" => false,
    ],
  ],
  "Sampler" => [
    "spry.make_sampler" => [
      "desc" => "
        Create a sampler used with images and fonts.

        `min_filter` and `mag_filter` can be set to:
        - `linear`, good for pixel art
        - `nearest`, suitable for larger/smoother images

        `wrap_u` and `wrap_v` can be set to `repeat`, `mirroredrepeat`, or
        `clamp`.
      ",
      "example" => "
        my_sampler = spry.make_sampler {
          min_filter = 'linear',
          mag_filter = 'linear',
          wrap_u = 'clamp',
          wrap_v = 'clamp',
        }
      ",
      "args" => [
        "t" => ["table", "Sampler options."],
        " .min_filter" => ["string", "Filter mode when scaling down."],
        " .mag_filter" => ["string", "Filter mode when scaling up."],
        " .wrap_u" => ["string", "Wrap mode for the horizontal direction."],
        " .wrap_v" => ["string", "Wrap mode for the vertical direction."],
      ],
      "return" => "Sampler",
    ],
    "spry.default_sampler" => [
      "desc" => "
        Get the default sampler, which uses nearest neighbor filtering, and
        repeat wrapping.
      ",
      "example" => "
        spry.default_sampler():use()
        player:draw(x, y, 0, sx, sy)
      ",
      "args" => [],
      "return" => "Sampler",
    ],
    "Sampler:use" => [
      "desc" => "Use this sampler for future image/font drawing.",
      "example" => "
        wrap_sampler:use()
        background:draw(x, y, r, sx, sy, ox, oy, u0, v0, u1, v1)
      ",
      "args" => [],
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
      "return" => [
        "on success" => "Image",
        "if image can't be loaded" => "nil",
      ],
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
      "return" => [
        "on success" => "Font",
        "if font can't be loaded" => "nil",
      ],
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
      "desc" => "
        Draw text onto the screen. Returns the bottom y position of the text.
        If `limit` is positive, words wrap by `limit` width per line.
      ",
      "example" => "font:draw('Hello, World!', 100, 100, 30)",
      "args" => [
        "text" => ["string", "The text to draw."],
        "x" => ["number", "The x position to draw at.", 0],
        "y" => ["number", "The y position to draw at.", 0],
        "size" => ["number", "The size of the text.", 12],
        "limit" => ["number", "Word wrap width.", -1],
      ],
      "return" => "number",
    ],
  ],
  "Sound" => [
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
    "spry.sound_load" => [
      "desc" => "Create a sound source from a file.",
      "example" => "local sound = spry.sound_load 'click.ogg'",
      "args" => [
        "file" => ["string", "The audio file to open."],
      ],
      "return" => [
        "on success" => "Sound",
        "if sound can't be loaded" => "nil",
      ],
    ],
    "Sound:frames" => [
      "desc" => "Returns the length of the sound in PCM frames.",
      "example" => "local frames = sound:frames()",
      "args" => [],
      "return" => "number",
    ],
    "Sound:secs" => [
      "desc" => "Returns the length of the sound in seconds.",
      "example" => "local len = sound:secs()",
      "args" => [],
      "return" => "number",
    ],
    "Sound:start" => [
      "desc" => "Start audio playback for this sound.",
      "example" => "
        if enemy:hurt() then
          grunt_sound:start()
        end
      ",
      "args" => [],
      "return" => false,
    ],
    "Sound:stop" => [
      "desc" => "Stop audio playback for this sound.",
      "example" => "
        if enter_door then
          bgm:stop()
        end
      ",
      "args" => [],
      "return" => false,
    ],
    "Sound:seek" => [
      "desc" => "Seek to the given PCM frame.",
      "example" => "
        if music_restart then
          music:seek(0)
          music:start()
        end
      ",
      "args" => [
        "f" => ["number", "The PCM frame."],
      ],
      "return" => false,
    ],
    "Sound:vol" => [
      "desc" => "Get the sound's volume in the range [0, 1].",
      "example" => "local v = sound:vol()",
      "args" => [],
      "return" => "number",
    ],
    "Sound:set_vol" => [
      "desc" => "Set the sound's volume.",
      "example" => "local v = sound:vol()",
      "args" => [
        "vol" => ["number", "The volume in the range [0, 1]."],
      ],
      "return" => false,
    ],
    "Sound:pan" => [
      "desc" => "Get the sound's panning value in the range [-1, 1].",
      "example" => "local v = sound:pan()",
      "args" => [],
      "return" => "number",
    ],
    "Sound:set_pan" => [
      "desc" => "Set the sound's panning value.",
      "example" => "local v = sound:pan()",
      "args" => [
        "pan" => ["number", "The panning in the range [-1, 1]."],
      ],
      "return" => false,
    ],
    "Sound:pitch" => [
      "desc" => "Get the sound's pitch.",
      "example" => "local v = sound:pitch()",
      "args" => [],
      "return" => "number",
    ],
    "Sound:set_pitch" => [
      "desc" => "Set the sound's pitch. A pitch of 1 does not affect the sound.",
      "example" => "
        if slow_mo then
          sound:set_pitch(0.5)
        end
      ",
      "args" => [
        "pitch" => ["number", "The pitch value. Should be greater than 0."],
      ],
      "return" => false,
    ],
    "Sound:loop" => [
      "desc" => "Returns true if the sound is looping.",
      "example" => "local is_loop = sound:loop()",
      "args" => [],
      "return" => "boolean",
    ],
    "Sound:set_loop" => [
      "desc" => "Set sound looping.",
      "example" => "
        local bgm = music_audio:make_sound()
        bgm:set_loop()
        bgm:start()
      ",
      "args" => [
        "loop" => ["boolean", "True if the sound should loop."],
      ],
      "return" => false,
    ],
    "Sound:pos" => [
      "desc" => "Get the sound's position.",
      "example" => "local x, y = sound:pos()",
      "args" => [],
      "return" => "number, number",
    ],
    "Sound:set_pos" => [
      "desc" => "Set the sound's position.",
      "example" => "
        function Car:update()
          self.engine_sound:set_pos(self.x, self.y)
        end
      ",
      "args" => [
        "x" => ["number", "The x position of the sound in the range [-1, 1]."],
        "y" => ["number", "The y position of the sound in the range [-1, 1]."],
      ],
      "return" => false,
    ],
    "Sound:vel" => [
      "desc" => "Get the sound's velocity.",
      "example" => "local x, y = sound:vel()",
      "args" => [],
      "return" => "number, number",
    ],
    "Sound:set_vel" => [
      "desc" => "Set the sound's velocity used for doppler effect.",
      "example" => "whoosh:set_vel(self.x, self.y)",
      "args" => [
        "x" => ["number", "The x velocity of the sound."],
        "y" => ["number", "The y velocity of the sound."],
      ],
      "return" => false,
    ],
    "Sound:set_fade" => [
      "desc" => "Fade a sound in/out.",
      "example" => "wind:set_fade(-1, 0, 1000)",
      "args" => [
        "from" => ["number", "Starting volume. Set to -1 to use the current volume."],
        "to" => ["number", "Ending volume."],
        "ms" => ["number", "Number of milliseconds to fade."],
      ],
      "return" => false,
    ],
  ],
  "Sprite" => [
    "spry.sprite_load" => [
      "desc" => "
        Create a sprite from an Aseprite file. Sprite data is cached, so
        calling this multiple tiles for the same file is cheap.
      ",
      "example" => "
        function Player:new()
          self.sprite = spry.sprite_load 'player.ase'
        end
      ",
      "args" => [
        "file" => ["string", "The Aseprite file to open."],
      ],
      "return" => [
        "on success" => "Sprite",
        "if sprite can't be loaded" => "nil",
      ],
    ],
    "Sprite:play" => [
      "desc" => "
        Play an animation loop with the given tag. If the sprite's animation
        loop is the same as the previous, then this function does nothing,
        unless restart is also set to true.
      ",
      "example" => "
        if spry.key_down 'w' then
          spr:play 'walk_up'
        end
      ",
      "args" => [
        "tag" => ["string", "The tag name."],
        "restart" => ["boolean", "Force the animation to start on the first frame of the loop."],
      ],
      "return" => false,
    ],
    "Sprite:update" => [
      "desc" => "Call this method every frame to update a sprite's animation state.",
      "example" => "spr:update(dt)",
      "args" => [
        "dt" => ["number", "Delta time."],
      ],
      "return" => false,
    ],
    "Sprite:draw" => [
      "desc" => "Draw a sprite on the screen.",
      "example" => "spr:draw(x, y)",
      "args" => $draw_description,
      "return" => false,
    ],
    "Sprite:width" => [
      "desc" => "Get the width of a sprite in pixels.",
      "example" => "local w = spr:width()",
      "args" => [],
      "return" => "number",
    ],
    "Sprite:height" => [
      "desc" => "Get the height of a sprite in pixels.",
      "example" => "local h = spr:height()",
      "args" => [],
      "return" => "number",
    ],
    "Sprite:set_frame" => [
      "desc" => "Set the current frame index for a sprite.",
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
    "Sprite:total_frames" => [
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
      "return" => [
        "on success" => "Atlas",
        "if atlas can't be loaded" => "nil",
      ],
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
      "return" => [
        "on success" => "Tilemap",
        "if tilemap can't be loaded" => "nil",
      ],
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
      "desc" => "Draw all box fixtures for a given tilemap layer.",
      "example" => "
        tilemap:draw_fixtures(b2, 'Collision')
      ",
      "args" => [
        "world" => ["b2World", "The Box2D physics world."],
        "layer" => ["string", "The name of the IntGrid collision layer."],
      ],
      "return" => false,
    ],
    "Tilemap:make_graph" => [
      "desc" => "
        Prepare a tilemap for pathfinding by internally constructing a graph.

        The `costs` argument is required. The keys are the IntGrid values and
        the values are the costs to traverse to each tile. Any IntGrid values
        not in the `costs` table will be ignored.

        The `bloom` argument determines the number of nodes to consider as
        neighbor nodes. For example, a bloom of 1 looks at adjacent tiles. A
        bloom of 2 looks for nodes in a 5x5 region, 2 nodes outwards. Bloom
        of 3 looks for nodes in a 7x7 region, 3 nodes outwards, etc.
      ",
      "example" => "
        tilemap = spry.tilemap_load 'map.ldtk'
        tilemap:make_graph('Floor', { [1] = 1, [2] = 1 }, 2)
      ",
      "args" => [
        "layer" => ["string", "The name of the IntGrid collision layer."],
        "costs" => ["table", "The costs to traverse to each tile."],
        "bloom" => ["number", "The number of nodes outwards to consider as neighbors.", 1],
      ],
      "return" => false,
    ],
    "Tilemap:astar" => [
      "desc" => "Find the shortest path between two tiles.",
      "example" => "
        local path = tilemap:astar(start.x, start.y, goal.x, goal.y)
        for k, v in ipairs(path) do
          spry.draw_filled_rect(v.x, v.y, 16, 16)
        end
      ",
      "args" => [
        "sx" => ["number", "The starting tile's x position."],
        "sy" => ["number", "The starting tile's y position."],
        "ex" => ["number", "The target tile's x position."],
        "ey" => ["number", "The target tile's y position."],
      ],
      "return" => "table",
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
  "LuaSocket" => [
    "require('socket')" => [
      "desc" => "
        [LuaSocket](https://lunarmodules.github.io/luasocket/introduction.html) module.
        Not available for web builds.
      ",
      "example" => "
        local socket = require 'socket'

        function spry.start()
          sock = socket.udp()
          sock:setpeername('127.0.0.1', 3000)
          sock:send 'Hello from client to server'
        end
      "
    ],
  ],
  "microui" => [
    "spry.microui" => [
      "desc" => "
        Namespace for the [microui](https://github.com/rxi/microui/tree/master) module.
        The API is similar to the one found in C.
        The [`microui.ref`](#microui.ref) function is used for controls that
        used to rely on pointers.

        This namespace contains following constants:

        `VERSION`

        `COMMANDLIST_SIZE`, `ROOTLIST_SIZE`, `CONTAINERSTACK_SIZE`,
        `CLIPSTACK_SIZE`, `IDSTACK_SIZE`, `LAYOUTSTACK_SIZE`,
        `CONTAINERPOOL_SIZE`, `TREENODEPOOL_SIZE`, `MAX_WIDTHS`, `REAL_FMT`,
        `SLIDER_FMT`, `MAX_FMT`

        `CLIP_PART`, `CLIP_ALL`

        `COMMAND_JUMP`, `COMMAND_CLIP`, `COMMAND_RECT`, `COMMAND_TEXT`,
        `COMMAND_ICON`

        `COLOR_TEXT`, `COLOR_BORDER`, `COLOR_WINDOWBG`, `COLOR_TITLEBG`,
        `COLOR_TITLETEXT`, `COLOR_PANELBG`, `COLOR_BUTTON`,
        `COLOR_BUTTONHOVER`, `COLOR_BUTTONFOCUS`, `COLOR_BASE`,
        `COLOR_BASEHOVER`, `COLOR_BASEFOCUS`, `COLOR_SCROLLBASE`,
        `COLOR_SCROLLTHUMB`

        `ICON_CLOSE`, `ICON_CHECK`, `ICON_COLLAPSED`, `ICON_EXPANDED`

        `RES_ACTIVE`, `RES_SUBMIT`, `RES_CHANGE`

        `OPT_ALIGNCENTER`, `OPT_ALIGNRIGHT`, `OPT_NOINTERACT`, `OPT_NOFRAME`,
        `OPT_NORESIZE`, `OPT_NOSCROLL`, `OPT_NOCLOSE`, `OPT_NOTITLE`,
        `OPT_HOLDFOCUS`, `OPT_AUTOSIZE`, `OPT_POPUP`, `OPT_CLOSED`,
        `OPT_EXPANDED`
      ",
      "example" => "
        function spry.start()
          mu = spry.microui
        end

        function spry.frame(dt)
          if mu.begin_window('Test Window', mu.rect(40, 40, 300, 400)) then
            if mu.header('Test Buttons', mu.OPT_EXPANDED) then
              mu.layout_row({-1}, 0)
              mu.label 'Test buttons 1:'
              mu.layout_row({150, -1}, 0)
              if mu.button 'Button 1' then print 'Pressed button 1' end
              if mu.button 'Button 2' then print 'Pressed button 2' end
            end

            mu.end_window()
          end
        end
      ",
    ],
    "microui.set_focus" => [
      "args" => [ "id" => ["number"] ],
      "return" => false,
    ],
    "microui.get_id" => [
      "args" => [ "name" => ["string"] ],
      "return" => "number",
    ],
    "microui.push_id" => [
      "args" => [ "name" => ["string"] ],
      "return" => false,
    ],
    "microui.pop_id" => [
      "args" => [],
      "return" => false,
    ],
    "microui.push_clip_rect" => [
      "args" => $mu_rect,
      "return" => false,
    ],
    "microui.pop_clip_rect" => [
      "args" => [],
      "return" => false,
    ],
    "microui.get_clip_rect" => [
      "desc" => "Returns the clip rect with `x`, `y`, `w`, `h` keys.",
      "args" => [],
      "return" => "table",
    ],
    "microui.check_clip" => [
      "desc" => "Check clip result by using `mu.CLIP_PART` and `mu.CLIP_ALL`.",
      "args" => $mu_rect,
      "return" => "number",
    ],
    "microui.get_current_container" => [
      "args" => [],
      "return" => "mu_Container",
    ],
    "microui.get_container" => [
      "args" => [ "name" => ["string"] ],
      "return" => "mu_Container",
    ],
    "microui.bring_to_front" => [
      "args" => [ "container" => ["mu_Container"] ],
      "return" => false,
    ],
    "microui.set_clip" => [
      "args" => $mu_rect,
      "return" => false,
    ],
    "microui.draw_rect" => [
      "args" => array_merge($mu_rect, $mu_color),
      "return" => false,
    ],
    "microui.draw_box" => [
      "args" => array_merge($mu_rect, $mu_color),
      "return" => false,
    ],
    "microui.draw_text" => [
      "args" => array_merge([
        "str" => ["string"],
        "x" => ["number"],
        "y" => ["number"],
      ], $mu_color),
      "return" => false,
    ],
    "microui.draw_icon" => [
      "args" => array_merge([ "id" => ["number"] ], $mu_rect, $mu_color),
      "return" => false,
    ],
    "microui.layout_row" => [
      "desc" => "
        Similar to C's version of `mu_layout_row`, takes two arguments instead
        of three.
      ",
      "args" => [
        "widths" => ["table", "Widths of each item in the row."],
        "height" => ["number", "Height of the row."],
      ],
      "return" => false,
    ],
    "microui.layout_width" => [
      "args" => [ "width" => ["number"] ],
      "return" => false,
    ],
    "microui.layout_height" => [
      "args" => [ "height" => ["number"] ],
      "return" => false,
    ],
    "microui.layout_begin_column" => [
      "args" => [],
      "return" => false,
    ],
    "microui.layout_end_column" => [
      "args" => [],
      "return" => false,
    ],
    "microui.layout_set_next" => [
      "args" => array_merge($mu_rect, [ "relative" => ["boolean"] ]),
      "return" => false,
    ],
    "microui.draw_control_frame" => [
      "args" => array_merge(
        [ "id" => ["number"] ],
        $mu_rect,
        [ "colorid" => ["number"], "opt" => ["number"] ]
      ),
      "return" => false,
    ],
    "microui.draw_control_text" => [
      "args" => array_merge(
        [ "str" => ["string"] ],
        $mu_rect,
        [ "colorid" => ["number"], "opt" => ["number"] ]
      ),
      "return" => false,
    ],
    "microui.mouse_over" => [
      "args" => $mu_rect,
      "return" => "boolean",
    ],
    "microui.update_control" => [
      "args" => array_merge(
        [ "id" => ["number"] ],
        $mu_rect,
        [ "opt" => ["number"] ]
      ),
      "return" => false,
    ],
    "microui.text" => [
      "args" => [ "text" => ["string"] ],
      "return" => false,
    ],
    "microui.label" => [
      "args" => [ "text" => ["string"] ],
      "return" => false,
    ],
    "microui.button" => [
      "args" => [
        "text" => ["string"],
        "icon" => ["number", "", 0],
        "opt" => ["number", "", "microui.OPT_ALIGNCENTER"],
      ],
      "return" => "boolean",
    ],
    "microui.checkbox" => [
      "args" => [
        "text" => ["string"],
        "ref" => $mu_ref_desc,
      ],
      "return" => "number",
    ],
    "microui.textbox_raw" => [
      "args" => array_merge(
        [ "ref" => $mu_ref_desc, "id" => ["number"] ],
        $mu_rect,
        [ "opt" => ["number", "", 0] ]
      ),
      "return" => "number",
    ],
    "microui.textbox" => [
      "args" => [
        "ref" => $mu_ref_desc,
        "opt" => ["number", "", 0],
      ],
      "return" => "number",
    ],
    "microui.slider" => [
      "args" => [
        "ref" => $mu_ref_desc,
        "low" => ["number"],
        "high" => ["number"],
        "step" => ["number", "", 0],
        "fmt" => ["string", "", "microui.SLIDER_FMT"],
        "opt" => ["number", "", "microui.OPT_ALIGNCENTER"],
      ],
      "return" => "number",
    ],
    "microui.number" => [
      "args" => [
        "ref" => $mu_ref_desc,
        "step" => ["number"],
        "fmt" => ["string", "", "microui.SLIDER_FMT"],
        "opt" => ["number", "", "microui.OPT_ALIGNCENTER"],
      ],
      "return" => "number",
    ],
    "microui.header" => [
      "args" => [
        "text" => ["string"],
        "opt" => ["number", "", 0],
      ],
      "return" => "boolean",
    ],
    "microui.header" => [
      "args" => [
        "text" => ["string"],
        "opt" => ["number", "", 0],
      ],
      "return" => "boolean",
    ],
    "microui.begin_treenode" => [
      "args" => [
        "label" => ["string"],
        "opt" => ["number", "", 0],
      ],
      "return" => "boolean",
    ],
    "microui.end_treenode" => [
      "args" => [],
      "return" => false,
    ],
    "microui.begin_window" => [
      "args" => array_merge(
        [ "title" => ["string"] ],
        $mu_rect,
        [ "opt" => ["number", "", 0] ]
      ),
      "return" => "boolean",
    ],
    "microui.end_window" => [
      "args" => [],
      "return" => false,
    ],
    "microui.open_popup" => [
      "args" => [ "name" => ["string"] ],
      "return" => false,
    ],
    "microui.begin_popup" => [
      "args" => [ "name" => ["string"] ],
      "return" => "boolean",
    ],
    "microui.end_popup" => [
      "args" => [],
      "return" => false,
    ],
    "microui.end_popup" => [
      "args" => [],
      "return" => false,
    ],
    "microui.begin_panel" => [
      "args" => [
        "name" => ["string"],
        "opt" => ["number", "", 0],
      ],
      "return" => false,
    ],
    "microui.end_panel" => [
      "args" => [],
      "return" => false,
    ],
    "microui.get_hover" => [
      "desc" => "Accessor for `mu_Context.hover`.",
      "args" => [],
      "return" => "number",
    ],
    "microui.get_focus" => [
      "desc" => "Accessor for `mu_Context.focus`.",
      "args" => [],
      "return" => "number",
    ],
    "microui.get_last_id" => [
      "desc" => "Accessor for `mu_Context.last_id`.",
      "args" => [],
      "return" => "number",
    ],
    "microui.get_style" => [
      "desc" => "Accessor for `mu_Context.style`.",
      "args" => [],
      "return" => "mu_Style",
    ],
    "microui.rect" => [
      "desc" => "Create a table with `x`, `y`, `w`, `h` keys.",
      "example" => "
        local rect0 = spry.microui.rect(10, 10, 400, 300)
        local rect1 = { x = 10, y = 10, w = 400, h = 300 }

        assert(rect0.x == rect1.x)
        assert(rect0.y == rect1.y)
        assert(rect0.w == rect1.w)
        assert(rect0.h == rect1.h)
      ",
      "args" => [
        "x" => ["number", "The top left x position."],
        "y" => ["number", "The top left y position."],
        "w" => ["number", "The width of the rectangle."],
        "h" => ["number", "The height of the rectangle."],
      ],
      "return" => "table",
    ],
    "microui.color" => [
      "desc" => "Create a table with `r`, `g`, `b`, `a` keys.",
      "example" => "
        local col0 = spry.microui.col(128, 128, 128, 255)
        local col1 = { r = 128, g = 128, b = 128, a = 255 }

        assert(col0.r == col1.r)
        assert(col0.g == col1.g)
        assert(col0.b == col1.b)
        assert(col0.a == col1.a)
      ",
      "args" => [
        "r" => ["number", "The color's red channel, in the range [0, 255]."],
        "g" => ["number", "The color's green channel, in the range [0, 255]."],
        "b" => ["number", "The color's blue channel, in the range [0, 255]."],
        "a" => ["number", "The color's alpha channel, in the range [0, 255]."],
      ],
      "return" => "table",
    ],
  ],
  "microui Container" => [
    "mu_Container:rect" => [
      "desc" => "Get the container rectangle with `x`, `y`, `w`, `h` keys.",
      "args" => [],
      "return" => "table",
    ],
    "mu_Container:set_rect" => [
      "args" => $mu_rect,
      "return" => false,
    ],
    "mu_Container:body" => [
      "desc" => "Get the container body with `x`, `y`, `w`, `h` keys.",
      "args" => [],
      "return" => "table",
    ],
    "mu_Container:content_size" => [
      "args" => [],
      "return" => "number, number",
    ],
    "mu_Container:scroll" => [
      "args" => [],
      "return" => "number, number",
    ],
    "mu_Container:set_scroll" => [
      "args" => [
        "x" => ["number", "The x scroll position."],
        "y" => ["number", "The y scroll position."],
      ],
      "return" => false,
    ],
    "mu_Container:zindex" => [
      "args" => [],
      "return" => "number",
    ],
    "mu_Container:open" => [
      "args" => [],
      "return" => "boolean",
    ],
  ],
  "microui Style" => [
    "mu_Style:size" => [
      "args" => [],
      "return" => "number, number",
    ],
    "mu_Style:set_size" => [
      "args" => [ "x" => ["number"], "y" => ["number"] ],
      "return" => false,
    ],
    "mu_Style:padding" => [
      "args" => [],
      "return" => "number",
    ],
    "mu_Style:set_padding" => [
      "args" => [ "padding" => ["number"] ],
      "return" => false,
    ],
    "mu_Style:spacing" => [
      "args" => [],
      "return" => "number",
    ],
    "mu_Style:set_spacing" => [
      "args" => [ "spacing" => ["number"] ],
      "return" => false,
    ],
    "mu_Style:indent" => [
      "args" => [],
      "return" => "number",
    ],
    "mu_Style:set_indent" => [
      "args" => [ "indent" => ["number"] ],
      "return" => false,
    ],
    "mu_Style:title_height" => [
      "args" => [],
      "return" => "number",
    ],
    "mu_Style:set_title_height" => [
      "args" => [ "title_height" => ["number"] ],
      "return" => false,
    ],
    "mu_Style:scrollbar_size" => [
      "args" => [],
      "return" => "number",
    ],
    "mu_Style:set_scrollbar_size" => [
      "args" => [ "scrollbar_size" => ["number"] ],
      "return" => false,
    ],
    "mu_Style:thumb_size" => [
      "args" => [],
      "return" => "number",
    ],
    "mu_Style:set_thumb_size" => [
      "args" => [ "thumb_size" => ["number"] ],
      "return" => false,
    ],
    "mu_Style:color" => [
      "desc" => "Get the color with `r`, `g`, `b`, `a` keys using given id.",
      "args" => [ "colorid" => ["number"] ],
      "return" => "table",
    ],
    "mu_Style:set_color" => [
      "args" => array_merge([ "colorid" => ["number"] ], $mu_color),
      "return" => false,
    ],
  ],
  "microui Ref" => [
    "microui.ref" => [
      "desc" => "
        Returns userdata value that can be passed into several microui
        functions.
      ",
      "example" => "
        function spry.start()
          mu = spry.microui

          check = mu.ref(false)
          text = mu.ref 'Hello, World!'
          color = {
            r = mu.ref(90),
            g = mu.ref(95),
            b = mu.ref(100),
          }
        end

        function spry.frame(dt)
          local col = {
            r = color.r:get(),
            g = color.g:get(),
            b = color.b:get(),
            a = 255,
          }
          spry.clear_color(col.r, col.g, col.b, col.a)

          font:draw(text:get(), 400, 100, 24)

          if mu.begin_window('Window', mu.rect(40, 40, 300, 400)) then
            if mu.header('Inputs', mu.OPT_EXPANDED) then
              mu.checkbox('My Checkbox', check)
              mu.textbox(text)

              mu.layout_row({50, -1}, 0)
              mu.label 'Red:'; mu.slider(color.r, 0, 255)
              mu.label 'Green:'; mu.slider(color.g, 0, 255)
              mu.label 'Blue:'; mu.slider(color.b, 0, 255)
            end

            mu.end_window()
          end
        end
      ",
      "args" => [
        "value" => ["any", "A boolean, number, or string value."],
      ],
      "return" => "mu_Ref",
    ],
    "mu_Ref:get" => [
      "desc" => "Get the internal value of this object.",
      "example" => "
        function spry.start()
          mu = spry.microui
          text = mu.ref 'Hello, World!'
        end

        function spry.frame(dt)
          font:draw(text:get(), 400, 100, 24)
        end
      ",
      "args" => [],
      "return" => "any",
    ],
    "mu_Ref:set" => [
      "desc" => "Set the internal value of this object.",
      "example" => "
        if (mu.textbox(text) & mu.RES_SUBMIT) ~= 0 then
          mu.set_focus(mu.get_last_id())
          text:set ''
        end
      ",
      "args" => [
        "value" => ["any", "A boolean, number, or string value."],
      ],
      "return" => false,
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
      "example" => "pos = pos:lerp(other, t)",
      "args" => [
        "rhs" => ["vec2", "Another vector."],
        "t" => ["number", "The interpolation amount."],
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
      "return" => [
        "on success" => "table",
        "if actor doesn't exist" => "nil",
      ],
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
      "return" => [
        "on success" => "table",
        "if entity doesn't exist" => "nil",
      ],
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
      "example" => "
        function Thing:new()
          self.spring = Spring()
        end

        function Thing:on_hit()
          self.spring:pull(0.5)
        end

        function Thing:update(dt)
          self.spring:update(dt)
        end

        function Thing:draw()
          local sx = 1 + self.spring.x
          local sy = 1 - self.spring.x
          img:draw(self.x, self.y, 0, sx, sy)
        end
      ",
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
  "Object Oriented" => [
    "class" => [
      "desc" => "Create a new class.",
      "example" => "
        -- create a global table called Player
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

        function spry.start()
          img = spry.image_load 'player.png'

          -- create an instance of Player
          player = Player(200, 200)
          assert(getmetatable(player) == Player)
        end

        function spry.frame(dt)
          -- update position and draw
          player:update(dt)
          player:draw()
        end
      ",
      "args" => [
        "name" => ["string", "The name of the class."],
        "parent" => ["table", "the parent class to inherit from.", "Object"],
      ],
      "return" => false,
    ],
    "Object:__call" => [
      "desc" => "Call the object's constructor.",
      "example" => "
        class 'Enemy'

        function Enemy:new(x, y)
          self.x, self.y = x, y
        end

        e = Enemy(100, 200)
      ",
      "args" => [
        "..." => [false, "The arguments to pass to the constructor."],
      ],
      "return" => "table",
    ],
    "Object:is" => [
      "desc" => "Check if this object has the given metatable.",
      "example" => "
        class 'Car'
        class 'Animal'
        class('Cat', Animal)

        local cat = Cat()

        assert(cat:is(Animal))
        assert(cat:is(Cat))
        assert(not cat:is(Car))
      ",
      "args" => [
        "T" => ["table", "The metatable."],
      ],
      "return" => "boolean",
    ],
  ],
  "Utility Functions" => [
    "require" => [
      "desc" => "
        Loads a Lua module. This is not the same function in standard Lua. It
        ignores `package.searchers`, and it uses the virtual file system,
        searching for files relative to the root of the project directory or
        zip archive.

        When searching for files, path separators are `.` instead of `/`, and
        the `.lua` extension is excluded, similar to the standard version of
        this function.
      ",
      "example" => "
        local lume = require 'deps.lume'

        function spry.start()
          print(lume.serialize({ one = 1, arr = { 1, 2, 3 } }))
        end
      ",
      "args" => [
        "name" => ["string", "The module to load."],
      ],
      "return" => "any",
    ],
    "unsafe_require" => [
      "desc" => "
        This is the `require` function from standard Lua, renamed because it
        bypasses the virtual file system. This function can be used to load
        files in the `LUA_PATH` environment variable, for debugging
        purposes.
      ",
      "example" => "
        if os.getenv 'LOCAL_LUA_DEBUGGER_VSCODE' == '1' then
          unsafe_require 'lldebugger'.start()
        end
      ",
      "args" => [
        "name" => ["string", "The module to load."],
      ],
      "return" => "any",
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
      "desc" => "Get the dot product of two vectors.",
      "example" => "local dp = dot(vel_x, vel_y, other_x, other_y)",
      "args" => [
        "x0" => ["number", "The x component of a vector."],
        "y0" => ["number", "The y component of a vector."],
        "x1" => ["number", "The x component of another vector."],
        "y1" => ["number", "The y component of another vector."],
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
    "aabb_overlap" => [
      "desc" => "
        Checks if an AABB overlaps another AABB. An AABB is defined by its
        top, left, bottom, and right edges.
      ",
      "example" => "
        local overlap = aabb_overlap(
          left.x0, left.y0, left.x1, left.y1,
          right.x0, right.y0, right.x1, right.y1
        )
      ",
      "args" => [
        "ax0" => ["number", "First box's left position."],
        "ay0" => ["number", "First box's top position."],
        "ax1" => ["number", "First box's right position."],
        "ay1" => ["number", "First box's bottom position."],
        "bx0" => ["number", "Second box's left position."],
        "by0" => ["number", "Second box's top position."],
        "bx1" => ["number", "Second box's right position.", "bx0"],
        "by1" => ["number", "Second box's bottom position.", "by0"],
      ],
      "return" => "boolean",
    ],
    "rect_overlap" => [
      "desc" => "
        Checks if a rectangle overlaps another rectangle. A rectangle is
        defined by its top left point, width, and height.
      ",
      "example" => "
        local overlap = rect_overlap(b.x, b.y, b.w, b.h, spry.mouse_pos())
        local button_click = overlap and spry.mouse_click(0)
      ",
      "args" => [
        "ax" => ["number", "First rectangle's x position."],
        "ay" => ["number", "First rectangle's y position."],
        "aw" => ["number", "First rectangle's width."],
        "ah" => ["number", "First rectangle's height."],
        "bx" => ["number", "Second rectangle's x position."],
        "by" => ["number", "Second rectangle's y position."],
        "bw" => ["number", "Second rectangle's width.", 0],
        "bh" => ["number", "Second rectangle's height.", 0],
      ],
      "return" => "boolean",
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
    "find" => [
      "desc" => "
        Find the key in an array associated with the given value. Returns
        `nil` if value wasn't found.
      ",
      "example" => "
        local index = find(cards, 'queen:hearts')
        if index ~= nil then
          print('Queen of Hearts at index ' .. index)
        end
      ",
      "args" => [
        "arr" => ["table", "The array to search."],
        "x" => ["any", "The value to search for."],
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
<div x-cloak x-data="{ open: false }" class="mw8 center">
  <div x-show="open" @click="open = false" x-transition.opacity class="fixed absolute--fill bg-white-50 dm-bg-black-50 z-3 ml-100px"></div>
  <div
    x-show="!open"
    class="db dn-l fixed bottom-0 left-0 mb3 z-4 shadow ba bl-0 b--black-10 dm-b--white-20 bg-near-white dm-bg-near-black"
    style="border-top-right-radius: 0.5rem; border-bottom-right-radius: 0.5rem"
  >
    <button @click="open = true" class="bg-none bn pointer pa3 flex dark-gray dm-silver">
      <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" style="width: 24px; height: 24px">
        <path stroke-linecap="round" stroke-linejoin="round" d="M3.75 9h16.5m-16.5 6.75h16.5" />
      </svg>
    </button>
  </div>
  <div
    class="db-l fixed bottom-0 pl1 pr2 pb2 overflow-y-scroll overflow-x-hidden z-5 bg-near-white dm-bg-near-black translate-x-0-l"
    style="transition: transform 200ms; width: 70%; max-width: 300px; top: <?= NAV_HEIGHT ?>"
    :class="open ? '' : 'translate-x-n120px'"
  >
    <div class="top-0 bg-fade-down z-5 pb3" style="position: sticky">
      <button
        x-data="{ expand: false }"
        class="bn pointer f6 fw6 dark-gray dm-silver bg-none underline-hover pt3 pb1 ph1"
        @click="
          expand = !expand
          for (const detail of details) {
            detail.open = expand
            detail.dataset.open = expand
          }
        "
        x-text="expand ? 'Collapse all' : 'Expand all'"
      >Expand all</button>
      <form onsubmit="event.preventDefault()" class="flex items-center pt1">
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
    </div>
    <ul id="function-list" class="list pl1" style="margin-top: -1rem">
      <?php foreach ($api_reference as $header => $section): ?>
        <li class="mt3">
          <details>
            <summary class="fw6 pointer"><?= $header ?></summary>
            <ul class="list pl0 mt2">
              <?php foreach ($section as $name => $func): ?>
                <li class="pv1" data-key="<?= strtolower($name) ?>">
                  <a
                    href="#<?= strtolower($name) ?>"
                    class="function-link dark-gray dm-silver link underline-hover lh-solid dib"
                    @click="
                      open = false
                      $el.closest('details').dataset.open = 'true'
                    "
                  >
                    <code><?= $name ?></code>
                  </a>
                </li>
              <?php endforeach ?>
            </ul>
          </details>
        </li>
      <?php endforeach ?>
    </ul>
  </div>
  <div x-ignore class="pl3-l pt3 ml-300px-l">
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
                echo $name;
                if (isset($func["args"])) {
                  $args = array_keys($func["args"]);
                  $args = array_filter($args, fn($a) => !str_starts_with($a, " "));
                  echo "(" . implode(", ", $args) . ")";
                }
                ?>
              </code>
            </a>
          </h2>
          <?php if (isset($func["desc"])): ?>
            <div class="lh-copy prose">
              <?= Parsedown::instance()->text(multiline_trim($func["desc"])); ?>
            </div>
          <?php endif ?>
          <?php if (isset($func["example"])): ?>
            <div class="prose relative">
              <span class="dib absolute top-0 left-0 pt1 pl2 br3 gray fw6 f7 ttu">
                Example
              </span>
              <pre class="mv0"><code class="language-lua" style="padding-top: 1.5rem"><?= multiline_trim($func["example"]) ?></code></pre>
            </div>
          <?php endif ?>
          <?php if (isset($func["args"]) && count($func["args"]) > 0): ?>
            <?php
            $has_desc = false;
            $has_default = false;
            foreach ($func["args"] as $arr) {
              if (count($arr) >= 2 && $arr[1] !== "") { $has_desc = true; }
              if (count($arr) === 3) { $has_default = true; }
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
                      <?php if ($has_desc): ?>
                        <th class="tl pv2 ph3 fw6">Description</th>
                      <?php endif ?>
                    </tr>
                    <?php foreach ($func["args"] as $arg_name => $arg): ?>
                      <?php
                      $type = $arg[0];
                      $desc = $arg[1] ?? "";
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
                        <?php if ($has_desc): ?>
                          <td class="pv2 ph3"><?= $desc ?></td>
                        <?php endif ?>
                      </tr>
                    <?php endforeach ?>
                  </tbody>
                </table>
              </div>
            </div>
          <?php endif ?>

          <?php if (isset($func["return"])): ?>
            <?php if (gettype($func["return"]) === "string"): ?>
              <p class="mb0">
                <span class="mid-gray dm-moon-gray mt3 mb2 fw6">Returns</span>
                <code class="inline-code"><?= $func["return"] ?></code>.
              </p>
            <?php elseif (gettype($func["return"]) === "array"): ?>
              <div class="mt3">
                <span class="mid-gray dm-moon-gray mt3 mb2 fw6">Returns:</span>
                <?php foreach ($func["return"] as $kind => $ret): ?>
                  <p class="mt2 mb0">
                    <code class="inline-code"><?= $ret ?></code>
                    <span><?= $kind ?>.</span>
                  </p>
                <?php endforeach ?>
              </div>
            <?php else: ?>
              <p class="i gray mb0">Returns nothing.</p>
            <?php endif ?>
          <?php endif ?>
        </div>
      <?php endforeach ?>
      <hr class="bn bg-black-20 dm-bg-white-20 mv4" style="height: 1px">
    <?php endforeach ?>
    <div class="mv4 tr">
      <a href="https://github.com/jasonliang-dev/spry" class="gray link underline-hover pv3 dib">
        GitHub
      </a>
    </div>
  </div>
</div>

<script>
  const search = document.getElementById('search')
  const details = document.querySelectorAll('details')

  let expand = false
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

    if (search.value.length > 0 && !expand) {
      expand = true
      for (const detail of details) {
        detail.dataset.open = detail.open
        detail.open = true
      }
    } else if (search.value.length === 0 && expand) {
      expand = false
      for (const detail of details) {
        detail.open = detail.dataset.open === 'true'
      }
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
