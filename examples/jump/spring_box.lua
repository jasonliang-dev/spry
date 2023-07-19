class "SpringBox"

function SpringBox:new(x, y)
  self.x = x
  self.y = y

  self.is_up = false
  self.img_down = atlas:get_image "spring_down"
  self.img_up = atlas:get_image "spring_up"
end

function SpringBox:on_create()
  self.body = b2:make_static_body { x = self.x, y = self.y }
  self.body:make_box_fixture {
    w = 4,
    h = 2,
    udata = self.id,
    sensor = true,
  }
end

function SpringBox:on_death()
  self.body:destroy()
end

function SpringBox:update(dt)
  if self.y > max_height + 300 then
    world:kill(self)
  end
end

function SpringBox:draw()
  local x, y = self.body:position()

  local img = self.is_up and self.img_up or self.img_down

  local ox = img:width() * 0.5
  local oy = img:height() * 0.5
  img:draw(x, y - 1, 0, 0.5, 0.5, ox, oy)

  --self.body:draw_fixtures()
end
