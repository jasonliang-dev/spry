function spry.conf(t)
  t.swap_interval = 0
  t.window_width = 1280
  t.window_height = 720
end

function spry.start()
  font = spry.default_font()
  atlas = spry.atlas_load "atlas.rtpa"
  b2_world = spry.b2_world { gx = 0, gy = 9.81, meter = 80 }
  world = World()

  ground = {}
  ground.w = spry.window_width()
  ground.h = 10
  ground.body = b2_world:make_static_body {
    x = spry.window_width() / 2,
    y = spry.window_height(),
  }
  ground.body:make_box_fixture {
    w = ground.w + 10,
    h = ground.h,
    friction = 1,
    udata = "ground box",
  }

  wall = {}
  wall.w = 40
  wall.h = 80
  wall.body = b2_world:make_static_body {
    x = 300,
    y = spry.window_height() - wall.h - ground.h,
  }
  wall.body:make_box_fixture {
    w = wall.w,
    h = wall.h,
    friction = 1,
    udata = "wall box",
  }

  sensor_box = {}
  sensor_box.collisions = 0
  sensor_box.w = 80
  sensor_box.h = 60
  sensor_box.body = b2_world:make_static_body {
    x = 500,
    y = spry.window_height() - sensor_box.h - ground.h - 150,
  }
  sensor_box.body:make_box_fixture {
    w = sensor_box.w,
    h = sensor_box.h,
    sensor = true,
    friction = 1,
    udata = "sensor box",
  }

  b2_world:on_begin_contact(function (a, b)
    local sensor
    local other
    if a:udata() == "sensor box" then
      sensor, other = a, b
    elseif b:udata() == "sensor box" then
      sensor, other = b, a
    end

    if sensor ~= nil then
      sensor_box.collisions = sensor_box.collisions + 1
    end
  end)

  b2_world:on_end_contact(function (a, b)
    local sensor
    if a:udata() == "sensor box" then
      sensor = a
    elseif b:udata() == "sensor box" then
      sensor = b
    end

    if sensor ~= nil then
      sensor_box.collisions = sensor_box.collisions - 1
    end
  end)

  camera = { x = 0, y = 0 }
end

function spry.frame(dt)
  if spry.key_down "esc" then
    spry.quit()
  end

  local dx, dy = spry.mouse_delta()
  local moved = dx ~= 0 or dy ~= 0

  if spry.mouse_down(2) then
    camera.x = camera.x + dx
    camera.y = camera.y + dy
  end

  if moved and spry.mouse_down(0) or spry.mouse_click(0) then
    local mx, my = spry.mouse_pos()
    world:add(Box(mx - camera.x, my - camera.y))
  end

  if moved and spry.mouse_down(1) or spry.mouse_click(1) then
    local mx, my = spry.mouse_pos()
    world:add(Ball(mx - camera.x, my - camera.y))
  end

  b2_world:step(dt)
  world:update(dt)

  spry.clear_color(32, 32, 32, 255)
  spry.push_matrix()
    spry.translate(camera.x, camera.y)

    world:draw()

    ground.body:draw_fixtures()
    wall.body:draw_fixtures()
    if sensor_box.collisions > 0 then
      spry.push_color(255, 0, 0, 255)
    else
      spry.push_color(255, 255, 255, 255)
    end
    sensor_box.body:draw_fixtures()
    spry.pop_color()
  spry.pop_matrix()

  font:draw("Left click: box, Right click: ball", 10, 10)
  font:draw(("fps: %.2f"):format(1 / dt, dt * 1000), 10, 26)
end
