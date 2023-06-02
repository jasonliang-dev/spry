function spry.conf(t)
  t.swap_interval = false
  -- t.window_width = 1280
  -- t.window_height = 720
end

function spry.start()
  font = spry.default_font()
  tilemap = spry.tilemap_load "world.ldtk"
  sword_img = spry.image_load "sword.png"
  bow_img = spry.image_load "bow.png"
  arrow_img = spry.image_load "arrow.png"

  world = World()

  for k, v in ipairs(tilemap:entities()) do
    local mt = _G[v.id]
    if mt ~= nil then
      local obj = world:add(mt(v.x, v.y))
      if v.id == "Player" then
        player = obj
      end
    else
      print("no " .. v.id .. " class exists")
    end
  end

  cursor = Cursor(spry.image_load "cursor.png")

  camera = Camera {
    x = player.x,
    y = player.y,
    scale = 5,
  }

  debug_rects = {}

  spry.show_mouse(false)
end

function spry.frame(dt)
  if spry.key_down "esc" then
    spry.quit()
  end

  for k in ipairs(debug_rects) do
    debug_rects[k] = nil
  end

  world:update(dt)
  cursor:update(dt)

  local blend = 1 - 0.85 ^ (dt * 40)
  camera.x = lerp(camera.x, player.x, blend)
  camera.y = lerp(camera.y, player.y, blend)

  camera.scale = lerp(camera.scale, 2, dt ^ 0.8)

  spry.clear_color(48, 32, 32, 255)
  camera:begin_draw()
    tilemap:draw()
    world:draw()
    cursor:draw()

    for k, v in ipairs(debug_rects) do
      spry.draw_line_rect(v[1], v[2], v[3], v[4])
    end
  camera:end_draw()

  -- font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))

  -- local mx, my = spry.mouse_pos()
  -- local dx, dy = spry.mouse_delta()
  -- font:draw(("(%.2f, %.2f) (%.2f, %.2f)"):format(mx, my, dx, dy), 0, 24)
end
