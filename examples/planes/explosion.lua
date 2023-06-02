class "Explosion"

function Explosion:new()
  self.x = 0
  self.y = 0
  self.z = 50
  self.lifetime = 0
end

function Explosion:update(dt)
  self.lifetime = self.lifetime + dt

  if self.lifetime >= 0.1 then
    world:kill(self)
  end
end

function Explosion:draw()
  local tile = atlas:get_image "tile_0005"
  tile:draw(self.x, self.y, 0, 2, 2, tile:width() / 2, tile:height() / 2)
end