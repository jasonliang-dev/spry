function spry.conf(t)
  t.swap_interval = 0
end

function spry.start()
  ecs = ECS()

  for i = 1, 10 do
    ecs:add {
      pos = vec2(spry.window_width() / 2, spry.window_height() / 2),
      vel = vec2(random(-100, 100), random(-100, 100)),
      scale = 0.25,
      z_index = -1,
      img = spry.image_load "player.png",
    }
  end

  for i = 1, 10 do
    local id = ecs:add {
      pos = vec2(random(0, spry.window_width()), random(0, spry.window_height())),
      rot = { angle = random(0, math.pi), delta = random(-1, 1) },
      scale = 1,
      z_index = 0,
      img = spry.image_load "player.png",
    }

    ecs:add {
      pos = vec2(random(0, spry.window_width()), random(0, spry.window_height())),
      follow = id,
      scale = 0.5,
      z_index = -10,
      img = spry.image_load "player.png",
    }
  end
end

local function order_by_z(lhs, rhs)
  return lhs.entity.z_index < rhs.entity.z_index
end

function spry.frame(dt)
  ecs:update()

  for id, e in ecs:select { "pos", "vel" } do
    e.pos = e.pos + e.vel * vec2(dt, dt)

    if spry.key_down "p" then
      e.vel = nil
    end

    if spry.key_down "k" then
      ecs:kill(id)
    end
  end

  for id, e in ecs:select { "pos", "follow" } do
    local other = ecs:get(e.follow)
    if other ~= nil then
      e.pos = e.pos:lerp(other.pos, dt)
    end
  end

  for id, e in ecs:select { "pos", "rot" } do
    e.rot.angle = e.rot.angle + e.rot.delta * dt
  end

  for id, e in ecs:query {
    select = { "pos", "img", "z_index" },
    where = function(e)
      return e.pos.x < spry.window_width() / 2
    end,
    order_by = function(lhs, rhs)
      return lhs.entity.z_index < rhs.entity.z_index
    end,
  } do
    local ox = e.img:width() * 0.5
    local oy = e.img:height() * 0.5
    local angle = e.rot and e.rot.angle or 0
    local scale = e.scale or 1

    e.img:draw(e.pos.x, e.pos.y, angle, scale * 3, scale * 3, ox, oy)

    if e.vel == nil and spry.key_down "v" then
      e.vel = vec2(4, 1)
    end
  end

  spry.default_font():draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end