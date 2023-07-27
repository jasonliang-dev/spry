class "Player"

function Player:new(x, y)
  self.x, self.y = x, y
  self.img = spry.image_load "player.png"
end

function Player:update(dt)
  local vx, vy = 0, 0

  if spry.key_down "w" then vy = vy - 1 end
  if spry.key_down "s" then vy = vy + 1 end
  if spry.key_down "a" then vx = vx - 1 end
  if spry.key_down "d" then vx = vx + 1 end

  vx, vy = normalize(vx, vy)
  self.x = self.x + vx * dt * 300
  self.y = self.y + vy * dt * 300
end

function Player:draw()
  local ox = self.img:width() * 0.5
  local oy = self.img:height() * 0.5
  self.img:draw(self.x, self.y, 0, 3, 3, ox, oy)
end