function spry.conf(t)
  t.swap_interval = 1
  t.window_width = 800
  t.window_height = 600
end

function spry.start()
  font = spry.default_font()

  cursor = Cursor(spry.image_load "cursor.png")
  spry.show_mouse(false)

  b2 = spry.b2_world { gx = 0, gy = 0, meter = 16 }
  world = World()

  bow_img = spry.image_load "bow.png"
  arrow_img = spry.image_load "arrow.png"
  tilemap = spry.tilemap_load "map.ldtk"
  tilemap:make_collision(b2, "Collision", { 1 })

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

  camera = Camera {
    x = player.x,
    y = player.y,
    scale = 5,
  }

  draw_fixtures = false
  spry.clear_color(48, 32, 32, 255)
end

function spry.frame(dt)
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  if spry.key_press "tab" then
    draw_fixtures = not draw_fixtures
  end

  b2:step(dt)
  world:update(dt)
  cursor:update(dt)

  local blend = 1 - 0.85 ^ (dt * 40)
  camera.x = lerp(camera.x, player.x, blend)
  camera.y = lerp(camera.y, player.y, blend)
  camera.scale = lerp(camera.scale, 2, dt ^ 0.8)

  camera:begin_draw()
    tilemap:draw()
    world:draw()
    cursor:draw()

    if draw_fixtures then
      tilemap:draw_fixtures(b2, "Collision")
    end
  camera:end_draw()

  -- font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end
