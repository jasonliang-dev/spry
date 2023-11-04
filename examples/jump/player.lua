class "Player"

function Player:new(x, y)
  self.x = x
  self.y = y
  self.z_index = 10

  self.spin = 0
  self.sprite = spry.sprite_load "player.ase"
  self.facing_left = false
  self.spring = Spring(100, 14)
end

function Player:on_create()
  self.body = b2:make_dynamic_body {
    x = self.x,
    y = self.y,
  }

  self.body:make_box_fixture {
    w = 4,
    h = 1,
    udata = self.id,
    begin_contact = Player.begin_contact,
    sensor = true,
  }
end

function Player:on_death()
  self.body:destroy()
  game_over = true
end

function Player:update(dt)
  -- self.sprite:update(dt)
  self.spring:update(dt)

  self.x, self.y = self.body:position()

  if self.y < max_height then
    max_height = self.y
  elseif jump.death_barrier(self.y) then
    world:kill(self)
  end

  do
    local right, top = camera:to_world_space(spry.window_width(), spry.window_height())
    right = right + 8
    local left = -right
    if self.x < left then
      self.x = right
    elseif self.x > right then
      self.x = left
    end
    self.body:set_position(self.x, self.y)
  end

  local dx = 0
  do
    local left = spry.key_down "a" or spry.key_down "left"
    local right = spry.key_down "d" or spry.key_down "right"
    if left then dx = dx - 1 end
    if right then dx = dx + 1 end
  end

  self.body:apply_force(dx * 50, 0)

  local vx, vy = self.body:velocity()

  ---[[
  if vx > 5 then
    self.facing_left = false
  elseif vx < -5 then
    self.facing_left = true
  end
  --]]

  vx = clamp(vx, -80, 80)

  local blend = 1 - 0.85 ^ (dt * 20)
  vx = lerp(vx, 0, blend)

  self.body:set_velocity(vx, vy)

  self.spin = lerp(self.spin, 0, blend)
end

function Player:draw()
  local sx = math.cos(self.spin)
  sx = sx * (self.facing_left and 1 or -1)

  local sy = 1

  sx = sx - self.spring.x * 2 * sign(sx)
  sy = sy + self.spring.x * 2

  local ox = self.sprite:width() * 0.5
  local oy = self.sprite:height() * 0.5
  self.sprite:draw(self.x, self.y - 6, 0, sx, sy, ox, oy)
  --self.body:draw_fixtures()
end

function Player.begin_contact(a, b)
  local self = world:query_id(a:udata())
  local other = world:query_id(b:udata())
  local mt = getmetatable(other)

  local vx, vy = self.body:velocity()

  if mt == Platform or mt == MovingPlatform then
    if vy > 10 then
      self.body:set_velocity(vx, -150)
      self.spring:pull(0.2)
      audio_beep:make_sound():start()
    end

  elseif mt == SpringBox then
    if vy > 10 then
      self.spin = 25

      self.body:set_velocity(vx, -300)
      other.is_up = true
      audio_sparkle:make_sound():start()
    end

  elseif mt == Spikes then
    if vy > 15 then
      world:kill(self)
    end
  end
end
