class "Enemy"

function Enemy:new()
  self.spring = Spring()
  self.hp = 5
  self.x = random(0, spry.window_width())
  self.z = 5
  self.rocket_cooldown = 3

  if math.random() < 0.5 then
    self.y = spry.window_height()
    self.angle = -math.pi / 2
  else
    self.y = 0
    self.angle = math.pi / 2
  end
end

function Enemy:on_death()
  for i = 1, 10 do
    timeout(random(0, 0.2), function()
      local e = world:add(Explosion())
      e.x = self.x + random(-20, 20)
      e.y = self.y + random(-20, 20)
    end)
  end
end

function Enemy:update(dt)
  self.spring:update(dt)

  local enemies = world:query_mt(Enemy)
  for id, enemy in pairs(enemies) do
    if id ~= self.id then
      local dist = distance(self.x, self.y, enemy.x, enemy.y)
      if dist < 40 then
        world:kill(self)
      end
    end
  end

  self.rocket_cooldown = self.rocket_cooldown - dt
  if self.rocket_cooldown <= 0 then
    self.rocket_cooldown = self.rocket_cooldown + 5

    local r = world:add(Rocket())

    r.owner = self.id
    r.x = self.x
    r.y = self.y
    r.angle = self.angle
  end

  local angle = direction(self.x, self.y, player.x, player.y)
  local delta = delta_angle(self.angle, angle)
  self.angle = self.angle + sign(delta) * dt * 0.2

  local vx, vy = heading(self.angle, 50)

  self.x = self.x + vx * dt
  self.y = self.y + vy * dt

  local dist = distance(self.x, self.y, player.x, player.y)
  if dist < 40 then
    world:kill(self)
  end
end

function Enemy:draw()
  local ship = atlas:get_image "ship_0013"

  local r = self.angle + math.pi / 2
  local sx = 2 + self.spring.x
  local sy = 2 - self.spring.x
  local ox = ship:width() / 2
  local oy = ship:height() / 2

  if self.rocket_cooldown < 0.5 then
    spry.push_color(255, 192, 128, 255)
  end

  ship:draw(self.x, self.y, r, sx, sy, ox, oy)

  if self.rocket_cooldown < 0.5 then
    spry.pop_color()
  end
end

function Enemy:on_bullet_hit()
  self.spring:pull(0.6)
  self.hp = self.hp - 1
  if self.hp == 0 then
    world:kill(self)
  end
end