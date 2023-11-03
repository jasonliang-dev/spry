class "Animal"

function Animal:new(x, y)
  self.x, self.y = x, y
end

function Animal:say()
  font:draw(("%s has %d legs"):format(self.kind, self.legs), self.x, self.y, 24)
end
