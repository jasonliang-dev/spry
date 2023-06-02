class "Player"

function Player:new(x, y)
  self.x = x
  self.y = y

  self.sprite = spry.sprite_load "knight.ase"

  self.update_thread = create_thread(self.co_update)

  self.facing_left = false
  self.dir = 0

  self.swing_cooldown = 0
  self.swing_cooldown_max = 0.25
  self.swing_angle = 0
  self.swing_id = 0

  self.shoot_cooldown = 0
  self.shoot_cooldown_max = 0.3
  self.shoot_angle = 0
end

function Player:aabb()
  local ox = self.sprite:width() / 2
  local oy = self.sprite:height() * 0.75

  local x0 = self.x - ox
  local y0 = self.y - oy
  local x1 = self.x + ox
  local y1 = self.y

  return x0, y0, x1, y1
end

function Player:swinging()
  return self.swing_cooldown > 0
end

function Player:sword_aabb()
  local cos, sin = heading(self.swing_angle, 24)

  local size = 12
  local x0 = self.x + cos - size
  local y0 = self.y + sin - size - 8
  local x1 = self.x + cos + size
  local y1 = self.y + sin + size - 8

  return x0, y0, x1, y1
end

function Player:idle(dt)
  self.sprite:play "idle"

  while true do
    if
      spry.key_down "w" or
      spry.key_down "s" or
      spry.key_down "a" or
      spry.key_down "d"
    then
      self:run(dt)
      self.sprite:play "idle"
    end

    dt = yield()
  end
end

function Player:run(dt)
  self.sprite:play "run"

  while true do
    local vx, vy = 0, 0
    if spry.key_down "w" then vy = vy - 1 end
    if spry.key_down "s" then vy = vy + 1 end
    if spry.key_down "a" then vx = vx - 1 end
    if spry.key_down "d" then vx = vx + 1 end

    if vx == 0 and vy == 0 then
      return
    end

    vx, vy = normalize(vx, vy)

    if vx ~= 0 then
      self.facing_left = vx < 0
    end

    if vx ~= 0 or vy ~= 0 then
      self.dir = math.atan(vy, vx)
    end

    local mag = 100
    if self:swinging() then
      mag = 25
    end
    move_object(self, vx * mag * dt, vy * mag * dt, self:aabb())

    dt = yield()
  end
end

function Player:co_update(dt)
  self:idle(dt)
end

function Player:update(dt)
  self.z_index = self.y
  self.sprite:update(dt)
  resume(self.update_thread, self, dt)

  self.swing_cooldown = self.swing_cooldown - dt
  self.shoot_cooldown = self.shoot_cooldown - dt

  if spry.mouse_down(0) and not self:swinging() then
    self.swing_cooldown = self.swing_cooldown_max
    self.swing_angle = self.dir
    self.swing_id = self.swing_id + 1
  end

  if spry.mouse_down(1) and not self:swinging() and self.shoot_cooldown <= 0 then
    self.shoot_cooldown = self.shoot_cooldown_max

    local ox = self.x
    local oy = self.y - 8

    local mx, my = spry.mouse_pos()
    mx, my = camera:to_world_space(mx, my)

    local angle = direction(ox, oy, mx, my)
    self.shoot_angle = angle
    local dx, dy = heading(angle, 16)

    world:add(Arrow(ox + dx, oy + dy, angle))
  end
end

function Player:draw()
  local sx = self.facing_left and -1 or 1

  if self:swinging() then
    local cos = math.cos(self.swing_angle)
    if -1e-16 < cos and cos < 1e-16 then
      -- pass
    elseif cos < 0 then
      sx = -1
    elseif cos > 0 then
      sx = 1
    end

    local sword = sword_img

    local t = 1 - self.swing_cooldown / self.swing_cooldown_max

    -- local scale = lerp(0.9, 1.2, 4 * t * (1 - t))
    local scale = 1
    local range = (math.pi * 0.75) / 2
    local angle = self.swing_angle + lerp(-range, range, t)

    local ox = sword:width() / 2
    local oy = sword:height() + 8

    sword:draw(self.x, self.y - 8, angle + math.pi / 2, scale, scale, ox, oy)

    -- draw_debug_aabb(self:sword_aabb())
  end

  if self.shoot_cooldown > 0 then
    local cos = math.cos(self.shoot_angle)
    if -1e-16 < cos and cos < 1e-16 then
      -- pass
    elseif cos < 0 then
      sx = -1
    elseif cos > 0 then
      sx = 1
    end

    local bow = bow_img
    local ox = -bow_img:width() - 4
    local oy = bow_img:height() / 2
    bow:draw(self.x, self.y - 8, self.shoot_angle, 1, 1, ox, oy)
  end

  local ox = self.sprite:width() / 2
  local oy = self.sprite:height()
  self.sprite:draw(self.x, self.y, 0, sx, 1, ox, oy)
end