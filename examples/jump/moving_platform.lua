class "MovingPlatform"

function MovingPlatform:new(x, y)
  self.x = x
  self.y = y
  self.img = atlas:get_image "platform2"
end

function MovingPlatform:on_create()
  self.body = b2:make_kinematic_body {
    x = self.x,
    y = self.y,
    vx = choose {-30, 30, -40, 40, -50, 50},
  }

  self.body:make_box_fixture {
    w = 4,
    h = 2,
    udata = self.id,
    sensor = true,
  }
end

function MovingPlatform:on_death()
  self.body:destroy()
end

function MovingPlatform:update(dt)
  if jump.death_barrier(self.y) then
    world:kill(self)
  end

  self.x, self.y = self.body:position()

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

function MovingPlatform:draw()

  local ox = self.img:width() * 0.5
  local oy = self.img:height() * 0.5
  self.img:draw(self.x, self.y + 2, 0, 0.5, 0.5, ox, oy)
  --self.body:draw_fixtures()
end
