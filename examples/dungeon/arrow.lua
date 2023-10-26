class "Arrow"

function Arrow:new(x, y, angle)
  self.x = x
  self.y = y
  self.angle = angle
  self.lifetime = 0
end

function Arrow:on_create()
  local vx, vy = heading(self.angle, 500)

  self.body = b2:make_dynamic_body {
    x = self.x,
    y = self.y,
    vx = vx,
    vy = vy,
    angle = self.angle,
    fixed_rotation = true,
  }

  self.body:make_box_fixture {
    w = 8,
    h = 2,
    udata = self.id,
    begin_contact = Arrow.begin_contact,
  }
end

function Arrow:on_death()
  self.body:destroy()
end

function Arrow:update(dt)
  self.lifetime = self.lifetime + dt
  if self.lifetime > 10 then
    world:kill(self)
  end

  self.x, self.y = self.body:position()
end

function Arrow:draw()
  local img = arrow_img
  local ox = img:width() / 2
  local oy = img:height() / 2
  img:draw(self.x, self.y, self.angle + math.pi / 2, 1, 1, ox, oy)

  if draw_fixtures then
    self.body:draw_fixtures()
  end
end

function Arrow.begin_contact(a, b)
  local self = world:query_id(a:udata())
  local is_player = b:udata() == player.id

  if not is_player then
    world:kill(self)
  end
end