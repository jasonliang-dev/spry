class "Platform"

function Platform:new(x, y)
  self.x = x
  self.y = y
  self.img = atlas:get_image "platform"
end

function Platform:on_create()
  self.body = b2:make_static_body { x = self.x, y = self.y }
  self.body:make_box_fixture {
    w = 4,
    h = 2,
    udata = self.id,
    sensor = true,
  }
end

function Platform:on_death()
  self.body:destroy()
end

function Platform:update(dt)
  if jump.death_barrier(self.y) then
    world:kill(self)
  end
end

function Platform:draw()
  local ox = self.img:width() * 0.5
  local oy = self.img:height() * 0.5
  self.img:draw(self.x, self.y + 2, 0, 0.5, 0.5, ox, oy)
  --self.body:draw_fixtures()
end
