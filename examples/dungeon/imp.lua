class "Imp"

function Imp:new(x, y)
  self.x = x
  self.y = y
  self.sprite = spry.sprite_load "imp.ase"
  self.update_thread = coroutine.create(self.co_update)
  self.facing_left = false
end

function Imp:aabb()
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height()

  local x0 = self.x - ox
  local y0 = self.y - oy
  local x1 = self.x + ox
  local y1 = self.y

  return x0, y0, x1, y1
end

function Imp:idle(dt)
  self.sprite:play "idle"

  while true do
    local dist = distance(self.x, self.y, player.x, player.y)
    if dist < 128 then
      self:follow(dt)
      self.sprite:play "idle"
    end

    dt = yield()
  end
end

function Imp:follow(dt)
  self.sprite:play "run"

  while true do
    local vx = player.x - self.x
    local vy = player.y - self.y
    vx, vy = normalize(vx, vy)

    if vx ~= 0 then
      self.facing_left = vx < 0
    end

    move_object(self, vx * 60 * dt, vy * 60 * dt, self:aabb())

    if player:swinging() then
      local ax0, ay0, ax1, ay1 = player:sword_aabb()
      local bx0, by0, bx1, by1 = self:aabb()
      if ax0 < bx1 and bx0 < ax1 and ay0 < by1 and by0 < ay1 then
        world:kill(self)
      end
    end

    dt = yield()
  end
end

function Imp:co_update(dt)
  self:idle(dt)
end

function Imp:update(dt)
  self.sprite:update(dt)
  resume(self.update_thread, self, dt)
  self.z = self.y

  for id, arrow in pairs(world:query_mt(Arrow)) do
    local dist = distance(self.x, self.y - 8, arrow.x, arrow.y)
    if dist < 8 then
      world:kill(self)
      world:kill(arrow)
    end
  end
end

function Imp:draw()
  local sx = self.facing_left and -1 or 1
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height()
  self.sprite:draw(self.x, self.y, 0, sx, 1, ox, oy)
end
