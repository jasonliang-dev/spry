class "Smoke"

function Smoke:new()
  self.x = 0
  self.y = 0
  self.z = -1
  self.vx = 0
  self.vy = 0
  self.rotation = random(-1, 1)
  self.angle = 0
  self.lifetime = 0
  self.max_lifetime = 0.4
  self.scale = 1
end

function Smoke:update(dt)
  self.lifetime = self.lifetime + dt
  if self.lifetime >= self.max_lifetime then
    world:kill(self)
  end

  self.x = self.x + self.vx * dt
  self.y = self.y + self.vy * dt

  self.angle = self.angle + self.rotation * dt
end

function Smoke:draw()
  local tile = atlas:get_image "tile_0008"
  local fade = (1 - (self.lifetime / self.max_lifetime)) * 255
  spry.push_color(255, 255, 255, fade)
  tile:draw(self.x, self.y, self.angle, self.scale, self.scale, tile:width() / 2, tile:height() / 2)
  spry.pop_color()
end