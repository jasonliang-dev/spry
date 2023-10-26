class "Box"

function Box:new(x, y)
  local boxes = {
    "stone3",
    "stone3",
    "stone3",
    "stone3",
    "stone3",

    "stone4",
    "stone5",
    "stone6",
    "stone7",
    "stone8",
    "stone9",
  }

  self.img = atlas:get_image(choose(boxes))

  self.body = b2_world:make_dynamic_body { x = x, y = y, angle = random(0, math.pi * 2) }
  self.body:make_box_fixture {
    w = self.img:width() / 4 - 1,
    h = self.img:height() / 4 - 1,
    density = (self.img:width() * self.img:height()) / (70 * 70),
    friction = 0.3,
    udata = "box",
  }

  self.lifetime = 0
  self.max_lifetime = 3
end

function Box:on_death()
  self.body:destroy()
end

function Box:update(dt)
  self.lifetime = self.lifetime + dt
  if self.lifetime > self.max_lifetime then
    world:kill(self)
  end
end

function Box:draw()
  local x, y = self.body:position()
  local angle = self.body:angle()
  local ox = self.img:width() / 2
  local oy = self.img:height() / 2

  local alpha = lerp(255, 0, (self.lifetime / self.max_lifetime) ^ 20)
  spry.push_color(255, 255, 255, alpha)
  self.img:draw(x, y, angle, 0.5, 0.5, ox, oy)
  spry.pop_color()
end
