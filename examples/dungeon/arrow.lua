class "Arrow"

function Arrow:new(x, y, angle)
  self.x = x
  self.y = y
  self.angle = angle
end

function Arrow:aabb()
  local x0 = self.x - 4
  local y0 = self.y - 4
  local x1 = self.x + 4
  local y1 = self.y + 4

  return x0, y0, x1, y1
end

function Arrow:update(dt)
  local vx, vy = heading(self.angle, 400)
  local collision = move_object(self, vx * dt, vy * dt, self:aabb())
  if collision then
    world:kill(self)
  end
end

function Arrow:draw()
  local img = arrow_img
  local ox = img:width() / 2
  local oy = img:height() / 2
  img:draw(self.x, self.y, self.angle + math.pi / 2, 1, 1, ox, oy)
end