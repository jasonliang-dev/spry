class "Actor"

function Actor:new(path)
  assert(#path ~= 0)

  self.path = path
  self.index = 1
  self.x, self.y = tile_to_world(path[1].x, path[1].y)
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
  local tx, ty = tile_to_world(top.x, top.y)

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
  if self.dx ~= dx or self.dy ~= dy then
    if dx < 0 then
      self.sprite:play "walk_left"
    elseif dx > 0 then
      self.sprite:play "walk_right"
    elseif dy < 0 then
      self.sprite:play "walk_up"
    elseif dy > 0 then
      self.sprite:play "walk_down"
    end

    self.dx, self.dy = dx, dy
  end

  local s = dt * 100
  self.x = self.x + vx * s
  self.y = self.y + vy * s
end

function Actor:draw()
  local sx = 1 + self.spring.x
  local sy = 1 - self.spring.x
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height() / 2
  self.sprite:draw(self.x, self.y, 0, sx, sy, ox, oy)
end
