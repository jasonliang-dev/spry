function spry.conf(t)
  t.swap_interval = 0
end

function spry.start()
  font = spry.default_font()
  player = spry.image_load "player.png"

  ecs = ECS()

  for i = 1, 100 do
    ecs:add {
      pos = { x = 200, y = 200 },
      vel = { x = random(-100, 100), y = random(-100, 100) },
      img = player
    }
  end
end

function spry.frame(dt)
  ecs:update()

  for id, e in ecs:query { "pos", "vel" } do
    e.pos.x = e.pos.x + e.vel.x * dt
    e.pos.y = e.pos.y + e.vel.y * dt

    if spry.key_down "space" then
      e.vel = nil
    end

    if spry.key_down "k" then
      ecs:kill(id)
    end
  end

  for id, e in ecs:query { "pos", "img" } do
    local ox = e.img:width() * 0.5
    local oy = e.img:height() * 0.5
    e.img:draw(e.pos.x, e.pos.y, 0, 3, 3, ox, oy)

    if e.vel == nil and spry.key_down "j" then
      e.vel = { x = 4, y = 1 }
    end
  end

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end