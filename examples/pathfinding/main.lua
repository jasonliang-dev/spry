function spry.conf(t)
  t.swap_interval = 1
  t.target_fps = 0
end

function spry.start()
  font = spry.default_font()
  map = spry.tilemap_load "map.ldtk"
  map:make_graph('IntGrid', { [1] = 1 })
  start = { x = 0, y = 0 }
  goal = { x = 0, y = 0 }
end

function spry.frame(dt)
  local scale = 2

  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  local screen_to_tile = 1 / scale / 16

  if spry.mouse_down(0) then
    local x, y = spry.mouse_pos()
    start.x = x * screen_to_tile
    start.y = y * screen_to_tile
    path = map:astar(start.x, start.y, goal.x, goal.y)
  end

  if spry.mouse_down(1) then
    local x, y = spry.mouse_pos()
    goal.x = x * screen_to_tile
    goal.y = y * screen_to_tile
    path = map:astar(start.x, start.y, goal.x, goal.y)
  end

  if spry.mouse_down(2) then
    local x, y = spry.mouse_pos()
    local x = x * screen_to_tile
    local y = y * screen_to_tile
    neighbors = map:neighbors_for_tile(x, y)
  end

  spry.push_matrix()
  spry.scale(scale, scale)
    map:draw()

    if neighbors ~= nil then
      for k, v in ipairs(neighbors) do
        spry.draw_line_rect(v.x * 16 + 4, v.y * 16 + 4, 8, 8)
      end
    end

    if path ~= nil then
      for k, v in ipairs(path) do
        spry.push_color((#path - k) * 255 / #path, 0, k * 255 / #path, 255)
        spry.draw_line_rect(v.x * 16, v.y * 16, 16, 16)
        spry.pop_color()
      end
    end
  spry.pop_matrix()

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end
