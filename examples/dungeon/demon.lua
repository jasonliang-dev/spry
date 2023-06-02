class "Demon"

function Demon:new(x, y)
  self.x = x
  self.y = y
  self.sprite = spry.sprite_load "demon.ase"
  self.update_thread = create_thread(self.co_update)
  self.facing_left = false
  self.hp = 3
  self.spring = Spring()
  self.vx = 0
  self.vy = 0
  self.was_hit = false
  self.swing_id = 0
end

function Demon:on_death()
  for i = 1, 20 do
    local x = self.x + random(-16, 16)
    local y = self.y + random(-16, 16)
    world:add(Coin(x, y))
  end
end

function Demon:aabb()
  local px = 4
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height()

  local x0 = self.x - ox + px
  local y0 = self.y - oy
  local x1 = self.x + ox - px
  local y1 = self.y

  return x0, y0, x1, y1
end

function Demon:hit(vx, vy, mag)
  self.hp = self.hp - 1
  self.spring:pull(0.5)
  if self.hp == 0 then
    world:kill(self)
  end

  self.vx = vx
  self.vy = vy
  self.was_hit = true
end

function Demon:idle(dt)
  self.sprite:play "idle"

  while true do
    local dist = distance(self.x, self.y, player.x, player.y)
    if dist < 128 or self.was_hit then
      self:follow(dt)
      self.sprite:play "idle"
    end

    dt = yield()
  end
end

function Demon:follow(dt)
  self.sprite:play "run"

  while true do
    local vx = player.x - self.x
    local vy = player.y - self.y
    vx, vy = normalize(vx, vy)

    if vx ~= 0 then
      self.facing_left = vx < 0
    end

    move_object(self, vx * 40 * dt, vy * 40 * dt, self:aabb())

    dt = yield()
  end
end

function Demon:co_update(dt)
  self:idle(dt)
end

function Demon:update(dt)
  self.sprite:update(dt)
  self.spring:update(dt)

  resume(self.update_thread, self, dt)
  self.z_index = self.y

  local blend = 1 - 0.85 ^ (dt * 20)
  self.vx = lerp(self.vx, 0, blend)
  self.vy = lerp(self.vy, 0, blend)

  move_object(self, self.vx * dt, self.vy * dt, self:aabb())

  if player:swinging() and player.swing_id ~= self.swing_id then
    local ax0, ay0, ax1, ay1 = player:sword_aabb()
    local bx0, by0, bx1, by1 = self:aabb()
    if ax0 < bx1 and bx0 < ax1 and ay0 < by1 and by0 < ay1 then
      self.swing_id = player.swing_id
      local vx = self.x - player.x
      local vy = self.y - player.y
      local mag = 400
      vx, vy = normalize(vx, vy)
      self:hit(vx * mag, vy * mag)
    end
  end

  for id, arrow in pairs(world:query_mt(Arrow)) do
    local dist = distance(self.x, self.y - 16, arrow.x, arrow.y)
    if dist < 16 then
      local vx, vy = heading(arrow.angle, 100)
      self:hit(vx, vy)
      world:kill(arrow)
    end
  end
end

function Demon:draw()
  local sx = 1 - self.spring.x
  sx = self.facing_left and -sx or sx
  local sy = 1 + self.spring.x
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height()
  self.sprite:draw(self.x, self.y, 0, sx, sy, ox, oy)
end