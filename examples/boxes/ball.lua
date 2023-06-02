class "Ball"

function Ball:new(x, y, name)
  self.img = atlas:get_image "stone1"
  self.radius = self.img:width() / 4

  self.body = b2_world:make_dynamic_body { x = x, y = y }
  self.body:make_circle_fixture {
    radius = self.radius - 1,
    density = 1,
    friction = 0.3,
    restitution = 0.5,
    udata = "ball",
  }

  self.lifetime = 0
  self.max_lifetime = 3
end

function Ball:on_death()
  self.body:destroy()
end

function Ball:update(dt)
  self.lifetime = self.lifetime + dt
  if self.lifetime > self.max_lifetime then
    world:kill(self)
  end
end

function Ball:draw()
  local x, y = self.body:position()
  local angle = self.body:angle()
  local radius = self.radius
  local ox = radius * 2

  local alpha = lerp(255, 0, (self.lifetime / self.max_lifetime) ^ 20)
  spry.push_color(255, 255, 255, alpha)
  self.img:draw(x, y, angle, 0.5, 0.5, ox, ox)
  spry.pop_color()
end
