class "Coin"

function Coin:new(x, y)
  self.x = x
  self.y = y
  self.z = -1
  self.vx, self.vy = heading(random(0, math.pi * 2), 20)
  self.vz = random(-120, -330)
  self.spring = Spring()
  self.update_thread = create_thread(self.co_update)

  self.sprite = spry.sprite_load "coin.ase"
  self.sprite:set_frame(math.random(0, self.sprite:total_frames() - 1))
end

function Coin:co_update(dt)
  while true do
    self.vz = self.vz + dt * 800

    self.z = self.z + self.vz * dt
    self.x = self.x + self.vx * dt
    self.y = self.y + self.vy * dt

    if self.z > 0 then
      break
    end

    dt = yield()
  end

  self.z = 0
  self.spring:pull(0.6)
  dt = sleep_for(0.5, dt)

  repeat
    local dist = distance(self.x, self.y, player.x, player.y)
    dt = yield()
  until dist < 128

  repeat
    local blend = 1 - 0.5 ^ (dt * 40)
    self.x = lerp(self.x, player.x, blend)
    self.y = lerp(self.y, player.y, blend)

    local dist = distance(self.x, self.y, player.x, player.y)
    dt = yield()
  until dist < 8

  world:kill(self)
end

function Coin:update(dt)
  self.sprite:update(dt)
  self.spring:update(dt)
  resume(self.update_thread, self, dt)
end

function Coin:draw()
  local sx = 1 + self.spring.x
  local sy = 1 - self.spring.x
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height()
  self.sprite:draw(self.x, self.y + self.z, 0, sx, sy, ox, oy)
end