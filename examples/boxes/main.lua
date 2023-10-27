function spry.conf(t)
  t.swap_interval = 1
  t.window_width = 1280
  t.window_height = 720
end

function spry.start()
  font = spry.default_font()
  atlas = spry.atlas_load "atlas.rtpa"
  b2_world = spry.b2_world { gx = 0, gy = 9.81, meter = 80 }
  ecs = ECS()

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

  b2_world:begin_contact(function (a, b)
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

  b2_world:end_contact(function (a, b)
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
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  ecs:update()

  local dx, dy = spry.mouse_delta()
  local moved = dx ~= 0 or dy ~= 0

  if spry.mouse_down(2) then
    camera.x = camera.x + dx
    camera.y = camera.y + dy
  end

  if moved and spry.mouse_down(0) or spry.mouse_click(0) then
    local mx, my = spry.mouse_pos()

    local id, e = ecs:add {
      img = atlas:get_image(choose {
        "stone3",
        "stone3",
        "stone3",
        "stone3",
        "stone3",

        "stone4",
        "stone5",
        "stone6",
        "stone7",
        "stone8",
        "stone9",
      }),
      body = b2_world:make_dynamic_body {
        x = mx - camera.x,
        y = my - camera.y,
        angle = random(0, math.pi * 2),
      },
      life = {
        time = 0,
        max = 3,
      },
    }

    e.body:make_box_fixture {
      w = e.img:width() / 4 - 1,
      h = e.img:height() / 4 - 1,
      density = (e.img:width() * e.img:height()) / (70 * 70),
      friction = 0.3,
      udata = "box",
    }
  end

  if moved and spry.mouse_down(1) or spry.mouse_click(1) then
    local mx, my = spry.mouse_pos()

    local id, e = ecs:add {
      img = atlas:get_image "stone1",
      body = b2_world:make_dynamic_body {
        x = mx - camera.x,
        y = my - camera.y,
      },
      life = {
        time = 0,
        max = 3,
      },
    }

    e.body:make_circle_fixture {
      radius = e.img:width() / 4 - 1,
      density = 1,
      friction = 0.3,
      restitution = 0.5,
      udata = "ball",
    }
  end

  b2_world:step(dt)

  for id, e in ecs:select { 'body', 'life' } do
    e.life.time = e.life.time + dt
    if e.life.time > e.life.max then
      e.body:destroy()
      ecs:kill(id)
    end
  end

  spry.clear_color(32, 32, 32, 255)
  spry.push_matrix()
    spry.translate(camera.x, camera.y)

    for id, e in ecs:select { 'body', 'img', 'life' } do
      local x, y = e.body:position()
      local angle = e.body:angle()
      local ox = e.img:width() / 2
      local oy = e.img:height() / 2

      local alpha = lerp(255, 0, (e.life.time / e.life.max) ^ 20)
      spry.push_color(255, 255, 255, alpha)
      e.img:draw(x, y, angle, 0.5, 0.5, ox, oy)
      spry.pop_color()
    end

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

  if spry.platform() ~= "html5" then
    font:draw("Left click: box, Right click: ball", 10, 10)
    font:draw(("fps: %.2f"):format(1 / dt, dt * 1000), 10, 26)
  end
end
