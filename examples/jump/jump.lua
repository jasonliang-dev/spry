jump = {}

function jump:reset()
  max_height = 0
  next_platform = -20
  game_over = false

  if world then
    world:destroy_all()
  end

  if b2 then
    b2:destroy()
  end

  b2 = spry.b2_world { gx = 0, gy = 9.81, meter = 16 }
  world = World()

  world:add(Player(0, -50))
  world:add(Platform(0, 0))

  camera = Camera {
    x = 0,
    y = 0,
    scale = 5,
  }
end

function jump:update()
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
end

function jump.death_barrier(y)
  return y > max_height + 145
end
