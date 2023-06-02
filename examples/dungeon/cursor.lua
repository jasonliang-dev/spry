class "Cursor"

function Cursor:new(img)
  self.img = img
  self.angle = 0
end

function Cursor:update(dt)
  self.angle = self.angle + 2 * dt
end

function Cursor:draw()
  local x, y = spry.mouse_pos()
  if x == 0 and y == 0 then
    return
  end

  x, y = camera:to_world_space(x, y)

  local ox = self.img:width() / 2
  local oy = self.img:height() / 2
  self.img:draw(x, y, self.angle, 1, 1, ox, oy)
end
