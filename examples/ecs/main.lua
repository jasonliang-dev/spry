function spry.conf(t)
  t.swap_interval = 0
end

function spry.start()
  font = spry.default_font()
  player = spry.image_load "player.png"

  ecs = ECS()

  for i = 1, 50 do
    ecs:add {
      pos = { x = spry.window_width() / 2, y = spry.window_height() / 2 },
      vel = { x = random(-100, 100), y = random(-100, 100) },
      img = player,
    }
  end

  for i = 1, 10 do
    ecs:add {
      pos = { x = random(0, spry.window_width()), y = random(0, spry.window_height()) },
      rot = { angle = random(0, math.pi), delta = random(-1, 1) },
      img = player,
    }
  end
end

function spry.frame(dt)
  ecs:update()

  for id, e in ecs:query { "pos", "vel" } do
    e.pos.x = e.pos.x + e.vel.x * dt
    e.pos.y = e.pos.y + e.vel.y * dt

    if spry.key_down "p" then
      e.vel = nil
    end

    if spry.key_down "k" then
      ecs:kill(id)
    end
  end

  for id, e in ecs:query { "pos", "rot" } do
    e.rot.angle = e.rot.angle + e.rot.delta * dt
  end

  for id, e in ecs:query { "pos", "img" } do
    local ox = e.img:width() * 0.5
    local oy = e.img:height() * 0.5
    local angle = e.rot and e.rot.angle or 0

    e.img:draw(e.pos.x, e.pos.y, angle, 3, 3, ox, oy)

    if e.vel == nil and spry.key_down "v" then
      e.vel = { x = 4, y = 1 }
    end
  end

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end