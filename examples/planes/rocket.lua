class "Rocket"

function Rocket:new()
  self.x = 0
  self.y = 0
  self.angle = 0
  self.lifetime = 0

  self.timer = interval(0.1, function()
    local s = world:add(Smoke())
    s.x = self.x
    s.y = self.y
    s.scale = 1
    s.max_lifetime = 0.2
    s.vx, s.vy = heading(self.angle - math.pi, 50)
  end)
end

function Rocket:on_death()
  local e = world:add(Explosion())
  e.x = self.x
  e.y = self.y

  stop_interval(self.timer)
end

function Rocket:update(dt)
  self.lifetime = self.lifetime + dt
  if self.lifetime > 8 then
    world:kill(self)
  end

  local angle = direction(self.x, self.y, player.x, player.y)
  local delta = delta_angle(self.angle, angle)
  self.angle = self.angle + sign(delta) * dt * 0.8

  local vx, vy = heading(self.angle, 200)

  self.x = self.x + vx * dt
  self.y = self.y + vy * dt

  local rockets = world:query_mt(Rocket)
  for id, rocket in pairs(rockets) do
    if id ~= self.id then
      local dist = distance(self.x, self.y, rocket.x, rocket.y)
      if dist < 20 then
        world:kill(self)
      end
    end
  end

  local enemies = world:query_mt(Enemy)
  for id, enemy in pairs(enemies) do
    if id == self.owner then
      goto continue
    end

    local dist = distance(self.x, self.y, enemy.x, enemy.y)
    if dist < 40 then
      enemy:on_bullet_hit()
      world:kill(self)
    end

    ::continue::
  end

  local dist = distance(self.x, self.y, player.x, player.y)
  if dist < 40 then
    world:kill(self)
  end
end

function Rocket:draw()
  local tile = atlas:get_image "tile_0012"

  local r = self.angle + math.pi / 2
  local ox = tile:width() / 2
  local oy = tile:height() / 2

  tile:draw(self.x, self.y, r, 2, 2, ox, oy)
end
