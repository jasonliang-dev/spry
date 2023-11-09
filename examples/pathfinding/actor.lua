class "Actor"

function Actor:new(path)
  assert(#path ~= 0)

  self.path = path
  self.index = 1
  self.x, self.y = path[1].x, path[1].y
  self.vx, self.vy = 0, 0
  self.dx, self.dy = 0, 0
  self.sprite = spry.sprite_load "char.ase"
  self.spring = Spring()
  self.spring:pull(0.5)
end

function Actor:update(dt)
  self.spring:update(dt)
  self.sprite:update(dt)

  local top = self.path[self.index]
  local tx, ty = top.x, top.y

  if distance(self.x, self.y, tx, ty) < 2 then
    self.index = self.index + 1
    if self.index > #self.path then
      world:kill(self)
    end
  end

  local vx = tx - self.x
  local vy = ty - self.y
  vx, vy = normalize(vx, vy)

  local dx, dy = sign(vx), sign(vy)
  self.dx, self.dy = dx, dy

  if math.abs(vx) < 0.1 then
    if dy < 0 then
      self.sprite:play "walk_up"
    elseif dy > 0 then
      self.sprite:play "walk_down"
    end
  else
    if dx < 0 then
      self.sprite:play "walk_left"
    elseif dx > 0 then
      self.sprite:play "walk_right"
    end
  end

  local s = dt * 50
  self.x = self.x + vx * s
  self.y = self.y + vy * s
end

function Actor:draw()
  local x = self.x + tile / 2
  local y = self.y + tile / 2
  local sx = 1 + self.spring.x
  local sy = 1 - self.spring.x
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height() / 2
  self.sprite:draw(x, y, 0, sx, sy, ox, oy)
end
