function spry.conf(t)
  t.swap_interval = 1
  t.window_width = 540
  t.window_height = 960
  t.window_title = "Jump Game"
end

function spry.start()
  font = spry.default_font()
  atlas = spry.atlas_load "atlas.rtpa"

  b2 = spry.b2_world { gx = 0, gy = 9.81, meter = 16 }
  world = World()

  player = world:add(Player(0, -50))
  world:add(Platform(0, 0))

  max_height = 0
  next_platform = -20

  camera = Camera {
    x = 0,
    y = 0,
    scale = 5,
  }

  offset = {
    x = 0,
    y = 0,
  }

  bg_img = atlas:get_image "bg"
end

function spry.frame(dt)
  if spry.key_down "esc" then
    spry.quit()
  end

  b2:step(dt)
  world:update(dt)

  if player.y < max_height then
    max_height = player.y
  end

  while next_platform > max_height - 200 do
    local right, top = camera:to_world_space(spry.window_width(), spry.window_height())
    local left = -right

    local rx = random(left, right)

    local moving_chance
    if max_height < -4000 then
      moving_chance = 0.75
    elseif max_height < -2000 then
      moving_chance = 0.50
    elseif max_height < -1000 then
      moving_chance = 0.25
    else
      moving_chance = 0.10
    end

    local obj = (math.random() < moving_chance) and MovingPlatform or Platform
    world:add(obj(rx, next_platform))

    if (obj == Platform) and (math.random() < 0.1) then
      world:add(SpringBox(rx, next_platform - 6))
    end

    if (max_height < -1000) and (math.random() < 0.1) then
      local rx = random(left, right)
      local ry = next_platform + choose {-20, 20}
      world:add(Spikes(rx, ry - 5))
      world:add(Platform(rx, ry))
    end

    if max_height < -4000 then
      next_platform = next_platform - 60
    elseif max_height < -2000 then
      next_platform = next_platform - 40
    elseif max_height < -1000 then
      next_platform = next_platform - 30
    else
      next_platform = next_platform - 20
    end
  end

  local target_y = max_height
  local blend = 1 - 0.85 ^ (dt * 20)
  camera.y = lerp(camera.y, target_y, blend)

  do
    local vx, vy = 0, 0

    if spry.key_down "up" then vy = vy - 1 end
    if spry.key_down "down" then vy = vy + 1 end
    if spry.key_down "left" then vx = vx - 1 end
    if spry.key_down "right" then vx = vx + 1 end

    vx, vy = normalize(vx, vy)
    local mag = 1000
    offset.x = offset.x + vx * mag * dt
    offset.y = offset.y + vy * mag * dt
  end

  spry.clear_color(252, 223, 205, 255)

  spry.push_matrix()
    spry.translate(-offset.x, -offset.y)

    camera:begin_draw()
      world:draw()
    camera:end_draw()
  spry.pop_matrix()

  spry.push_color(0, 0, 0, 255)
    font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))

    font:draw(("height: %.2f"):format(max_height), 0, 12)

    text = ("%.0f"):format(-max_height)
    local text_size = 80
    local x = (spry.window_width() - font:width(text, text_size)) / 2
    font:draw(text, x, 100, text_size)
  spry.pop_color()
end
