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
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  if spry.mouse_down(0) then
    local x, y = spry.mouse_pos()
    start.x = x / 3 / 16
    start.y = y / 3 / 16
    path = map:astar(start.x, start.y, goal.x, goal.y)
  end

  if spry.mouse_down(1) then
    local x, y = spry.mouse_pos()
    goal.x = x / 3 / 16
    goal.y = y / 3 / 16
    path = map:astar(start.x, start.y, goal.x, goal.y)
  end

  if spry.mouse_down(2) then
    local x, y = spry.mouse_pos()
    local x = x / 3 / 16
    local y = y / 3 / 16
    neighbors = map:neighbors_for_tile(x, y)
  end

  spry.push_matrix()
  spry.scale(3, 3)
    map:draw()

    if neighbors ~= nil then
      for k, v in ipairs(neighbors) do
        spry.draw_line_rect(v.x * 16 + 4, v.y * 16 + 4, 8, 8)
      end
    end

    if path ~= nil then
      spry.push_color(255, 0, 0, 255)
      for k, v in ipairs(path) do
        spry.draw_line_rect(v.x * 16, v.y * 16, 16, 16)
      end
      spry.pop_color()
    end
  spry.pop_matrix()

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end
