class "Chort"

function Chort:new(x, y)
  self.x, self.y = x, y
  self.sprite = spry.sprite_load "enemy.ase"
  self.facing_left = false
  self.hit_cooldown = 0
  self.hp = 3
  self.spring = Spring()
  self.update_thread = create_thread(self.co_update)
end

function Chort:on_create()
  self.body = b2:make_dynamic_body {
    x = self.x,
    y = self.y,
    linear_damping = 10,
    fixed_rotation = true,
  }

  self.body:make_circle_fixture {
    y = -9,
    radius = 10,
    udata = self.id,
    begin_contact = Chort.begin_contact,
  }
  self.body:make_circle_fixture {
    y = -6,
    radius = 10,
    udata = self.id,
    begin_contact = Chort.begin_contact,
  }
end

function Chort:on_death()
  self.body:destroy()

  for i = 1, 8 do
    local x = self.x + random(-16, 16)
    local y = self.y + random(-16, 16)
    world:add(Coin(x, y))
  end
end

function Chort:hit(other)
  if self.hit_cooldown > 0 then
    return
  end

  self.hp = self.hp - 1
  if self.hp == 0 then
    world:kill(self)
  end

  self.hit_cooldown = 0.2

  self.body:set_velocity(heading(other.angle, 200))
  self.spring:pull(0.3)
end

function Chort:co_update(dt)
  self.sprite:play "chort_idle"
  while true do
    local dist = distance(self.x, self.y, player.x, player.y)
    if dist < 128 or self.hit_cooldown > 0 then
      break
    end

    dt = yield()
  end

  self.sprite:play "chort_run"
  while true do
    if self.hit_cooldown < 0 then
      local dx = player.x - self.x
      local dy = player.y - self.y
      dx, dy = normalize(dx, dy)
      self.facing_left = dx < 0

      local mag = 40
      self.body:set_velocity(dx * mag, dy * mag)
    end

    dt = yield()
  end
end

function Chort:update(dt)
  self.hit_cooldown = self.hit_cooldown - dt
  self.x, self.y = self.body:position()
  self.sprite:update(dt)
  self.spring:update(dt)
  resume(self.update_thread, self, dt)
end

function Chort:draw()
  local sx = 1 - self.spring.x
  local sy = 1 + self.spring.x

  sx = self.facing_left and -sx or sx

  local ox = self.sprite:width() / 2
  local oy = self.sprite:height()
  self.sprite:draw(self.x, self.y, 0, sx, sy, ox, oy)

  if draw_fixtures then
    self.body:draw_fixtures()
  end
end

function Chort.begin_contact(a, b)
  local self = world:query_id(a:udata())
  local other = world:query_id(b:udata())

  if other == nil then
    return
  end

  local mt = getmetatable(other)

  if mt == Player then
    world:kill(self)

  elseif mt == Arrow then
    self:hit(other)
  end
end
