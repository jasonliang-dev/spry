class "Skeleton"

function Skeleton:new(x, y)
  self.x, self.y = x, y
  self.sprite = spry.sprite_load "enemy.ase"
  self.facing_left = false
  self.was_hit = false
  self.hp = 2
  self.spring = Spring()

  self.sprite:play "skel_run"
end

function Skeleton:on_create()
  local vx, vy = heading(random(0, math.pi * 2), 70)

  self.body = b2:make_dynamic_body {
    x = self.x,
    y = self.y,
    vx = vx,
    vy = vy,
    fixed_rotation = true,
  }

  self.body:make_circle_fixture {
    y = -9,
    radius = 10,
    udata = self.id,
    restitution = 1,
    begin_contact = Skeleton.begin_contact,
  }
  self.body:make_circle_fixture {
    y = -6,
    radius = 10,
    udata = self.id,
    restitution = 1,
    begin_contact = Skeleton.begin_contact,
  }
end

function Skeleton:on_death()
  self.body:destroy()

  for i = 1, 5 do
    local x = self.x + random(-16, 16)
    local y = self.y + random(-16, 16)
    world:add(Coin(x, y))
  end
end

function Skeleton:hit(other)
  if self.was_hit then
    return
  end
  self.was_hit = true

  self.hp = self.hp - 1
  if self.hp == 0 then
    world:kill(self)
  end

  self.spring:pull(0.3)
end

function Skeleton:update(dt)
  self.x, self.y = self.body:position()
  self.sprite:update(dt)
  self.spring:update(dt)

  local vx, vy = self.body:velocity()
  if vx ~= 0 then
    self.facing_left = vx < 0
  end

  if self.was_hit then
    self.was_hit = false

    local mag = -70
    vx, vy = normalize(vx, vy)
    self.body:set_velocity(vx * mag, vy * mag)
  end
end

function Skeleton:draw()
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

function Skeleton.begin_contact(a, b)
  local self = world:query_id(a:udata())
  local other = world:query_id(b:udata())

  if other ~= nil then
    local mt = getmetatable(other)

    if mt == Player then
      world:kill(self)

    elseif mt == Arrow then
      self:hit(other)
    end
  end
end
