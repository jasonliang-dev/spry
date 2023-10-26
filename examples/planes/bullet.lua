class "Bullet"

function Bullet:new()
  self.x = 0
  self.y = 0
  self.angle = 0
  self.speed = 600
  self.lifetime = 0
end

function Bullet:on_death()
  local e = world:add(Explosion())
  e.x = self.x
  e.y = self.y
end

function Bullet:update(dt)
  self.lifetime = self.lifetime + dt
  if self.lifetime > 5 then
    world:kill(self)
  end

  local vx, vy = heading(self.angle, self.speed)

  self.x = self.x + vx * dt
  self.y = self.y + vy * dt

  local enemies = world:query_mt(Enemy)
  for id, enemy in pairs(enemies) do
    local dist = distance(self.x, self.y, enemy.x, enemy.y)
    if dist < 40 then
      enemy:on_bullet_hit()
      world:kill(self)
    end
  end

  local rockets = world:query_mt(Rocket)
  for id, rocket in pairs(rockets) do
    local dist = distance(self.x, self.y, rocket.x, rocket.y)
    if dist < 20 then
      world:kill(rocket)
      world:kill(self)
    end
  end
end

function Bullet:draw()
  local tile = atlas:get_image "tile_0000"

  local r = self.angle + math.pi / 2
  local ox = tile:width() / 2
  local oy = tile:height() / 2

  tile:draw(self.x, self.y, r, 2, 2, ox, oy)
end
