class "Chest"

function Chest:new(x, y)
  self.x = x
  self.y = y
  self.z_index = 0
  self.sprite = spry.sprite_load "chest.ase"

  self.sprite:play "closed"
end

function Chest:aabb()
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height() - 4

  local x0 = self.x - ox
  local y0 = self.y - oy
  local x1 = self.x + ox
  local y1 = self.y

  return x0, y0, x1, y1
end

function Chest:update(dt)
  self.sprite:update(dt)
end

function Chest:draw()
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height()
  self.sprite:draw(self.x, self.y, 0, 1, 1, ox, oy)
end
