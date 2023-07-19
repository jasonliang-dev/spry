class "Camera"

function Camera:new(def)
  self.x = def.x
  self.y = def.y
  self.scale = def.scale or 1
end

function Camera:to_world_space(x, y)
  x = x - spry.window_width() / 2
  y = y - spry.window_height() / 2

  x = x / camera.scale
  y = y / camera.scale

  x = x + camera.x
  y = y + camera.y

  return x, y
end

function Camera:to_screen_space(x, y)
  x = x + spry.window_width() / 2
  y = y + spry.window_height() / 2

  x = x * camera.scale
  y = y * camera.scale

  x = x - camera.x
  y = y - camera.y

  return x, y
end

function Camera:begin_draw()
  spry.push_matrix()
  spry.translate(spry.window_width() / 2, spry.window_height() / 2)
  spry.scale(self.scale, self.scale)
  spry.translate(-self.x, -self.y)
end

function Camera:end_draw()
  spry.pop_matrix()
end
