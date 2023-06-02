function draw_debug_aabb(x0, y0, x1, y1)
  push(debug_rects, {x0, y0, x1 - x0, y1 - y0})
end

function move_object(self, vx, vy, x0, y0, x1, y1)
  -- local chests = world:query_near(self.x, self.y, Chest)
  local chests = world:query_mt(Chest)
  local steps = 8

  vx = vx / steps
  vy = vy / steps

  local tpx = 2
  local tpb = 4

  local cpx = -3
  local cpt = 10

  y0 = y1 - 18

  -- draw_debug_aabb(x0 - tpx, y0, x1 + tpx, y1 + tpb)
  -- draw_debug_aabb(x0 - cpx, y0 + cpt, x1 + cpx, y1)

  local had_collision = false

  tilemap:grid_begin "Collision"
  for i = 1, steps do
    local collision = false

    if not tilemap:rect_every(0, x0 + vx - tpx, y0, x1 + vx + tpx, y1 + tpb) then
      collision = true
    end

    for id, chest in pairs(chests) do
      if collision then break end
      local ax0, ay0, ax1, ay1 = x0 + vx - cpx, y0 + cpt, x1 + vx + cpx, y1
      local bx0, by0, bx1, by1 = chest:aabb()
      if ax0 < bx1 and bx0 < ax1 and ay0 < by1 and by0 < ay1 then
        collision = true
      end
    end

    if not collision then
      self.x = self.x + vx
      x0 = x0 + vx
      x1 = x1 + vx
    else
      had_collision = true
    end

    collision = false

    if not tilemap:rect_every(0, x0 - tpx, y0 + vy, x1 + tpx, y1 + vy + tpb) then
      collision = true
    end

    for id, chest in pairs(chests) do
      if collision then break end
      local ax0, ay0, ax1, ay1 = x0 - cpx, y0 + vy + cpt, x1 + cpx, y1 + vy
      local bx0, by0, bx1, by1 = chest:aabb()
      if ax0 < bx1 and bx0 < ax1 and ay0 < by1 and by0 < ay1 then
        collision = true
      end
    end

    if not collision then
      self.y = self.y + vy
      y0 = y0 + vy
      y1 = y1 + vy
    else
      had_collision = true
    end
  end
  tilemap:grid_end()

  return had_collision
end
