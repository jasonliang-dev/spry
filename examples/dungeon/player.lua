class "Player"

function Player:new(x, y)
  self.x, self.y = x, y

  self.sprite = spry.sprite_load "elf.ase"

  self.update_thread = create_thread(self.co_update)

  self.facing_left = false

  self.shoot_cooldown = 0
  self.shoot_angle = 0
end

function Player:on_create()
  self.body = b2:make_dynamic_body {
    x = self.x,
    y = self.y,
    linear_damping = 25,
    fixed_rotation = true,
  }

  self.body:make_circle_fixture { y = -9, radius = 10, udata = self.id }
  self.body:make_circle_fixture { y = -6, radius = 10, udata = self.id }
end

function Player:on_death()
  self.body:destroy()
end

function Player:idle(dt)
  self.sprite:play "m_idle"

  while true do
    if
      spry.key_down "w" or
      spry.key_down "s" or
      spry.key_down "a" or
      spry.key_down "d"
    then
      self:run(dt)
      self.sprite:play "m_idle"
    end

    dt = yield()
  end
end

function Player:run(dt)
  self.sprite:play "m_run"

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

    local mag = 100
    self.body:set_velocity(vx * mag, vy * mag)

    dt = yield()
  end
end

function Player:co_update(dt)
  self:idle(dt)
end

function Player:update(dt)
  self.sprite:update(dt)
  self.x, self.y = self.body:position()
  resume(self.update_thread, self, dt)

  self.shoot_cooldown = self.shoot_cooldown - dt
  if spry.mouse_down(1) and self.shoot_cooldown <= 0 then
    self.shoot_cooldown = 0.4

    local ox = self.x
    local oy = self.y - 8

    local mx, my = camera:to_world_space(spry.mouse_pos())

    local angle = direction(ox, oy, mx, my)
    self.shoot_angle = angle
    local dx, dy = heading(angle, 16)

    world:add(Arrow(ox + dx, oy + dy, angle))
  end
end

function Player:draw()
  local sx = self.facing_left and -1 or 1

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
    local ox = -bow:width() - 4
    local oy = bow:height() / 2
    bow:draw(self.x, self.y - 8, self.shoot_angle, 1, 1, ox, oy)
  end

  local ox = self.sprite:width() / 2
  local oy = self.sprite:height()
  self.sprite:draw(self.x, self.y, 0, sx, 1, ox, oy)

  if draw_fixtures then
    self.body:draw_fixtures()
  end
end
