class "Player"

function Player:new()
  self.x = 100
  self.y = 100
  self.z = 10
  self.speed = 0
  self.angle = 0
  self.bullet_cooldown = 0
  self.spring = Spring()

  self.timer = interval(0.1, function()
    if self.speed > 100 then
      local s = world:add(Smoke())
      s.x = self.x
      s.y = self.y
      s.scale = 1.5
      s.vx, s.vy = heading(self.angle - math.pi, 100)
    end
  end)
end

function Player:on_death()
  stop_interval(self.timer)
end

function Player:update(dt)
  self.spring:update(dt)

  local turn_speed = 4
  local max_speed = 300
  if spry.key_down "space" then
    turn_speed = 1
    max_speed = 100
  end

  local blend = 1 - 0.85 ^ (dt * 20)
  if spry.key_down "w" then
    self.speed = lerp(self.speed, max_speed, blend)
  else
    self.speed = lerp(self.speed, 0, blend)
  end

  if spry.key_down "a" then self.angle = self.angle - dt * turn_speed end
  if spry.key_down "d" then self.angle = self.angle + dt * turn_speed end

  self.bullet_cooldown = self.bullet_cooldown - dt
  if spry.key_down "space" and self.bullet_cooldown <= 0 then
    self.bullet_cooldown = 0.15
    self.spring:pull(0.1)

    local vx, vy = heading(self.angle - math.pi, 5)
    self.x = self.x + vx
    self.y = self.y + vy

    local b = world:add(Bullet())
    b.x = self.x
    b.y = self.y
    b.angle = self.angle + random(-0.1, 0.1)
    b.speed = b.speed + self.speed
  end

  local vx, vy = heading(self.angle, self.speed)

  self.x = self.x + vx * dt
  self.y = self.y + vy * dt

  self.x = clamp(self.x, 0, spry.window_width())
  self.y = clamp(self.y, 0, spry.window_height())
end

function Player:draw()
  local ship = atlas:get_image "ship_0000"

  local r = self.angle + math.pi / 2
  local sx = 2 + self.spring.x
  local sy = 2 - self.spring.x
  local ox = ship:width() / 2
  local oy = ship:height() / 2

  ship:draw(self.x, self.y, r, sx, sy, ox, oy)
end