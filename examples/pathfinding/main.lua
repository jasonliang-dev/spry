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

  world = World()

  scale = 2
  tile = 16
end

function screen_to_tile(x, y)
  x = x / scale / tile
  y = y / scale / tile

  return x, y
end

function tile_to_world(x, y)
  x = x * tile + (tile / 2)
  y = y * tile + (tile / 2)

  return x, y
end

function spry.frame(dt)
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  if spry.mouse_down(0) then
    local x, y = spry.mouse_pos()
    start.x, start.y = screen_to_tile(x, y)
    path = map:astar(start.x, start.y, goal.x, goal.y)
    if spry.mouse_click(0) and #path > 0 then
      world:add(Actor(path))
    end
  end

  if spry.mouse_down(1) then
    local x, y = spry.mouse_pos()
    goal.x, goal.y = screen_to_tile(x, y)
    path = map:astar(start.x, start.y, goal.x, goal.y)
  end

  if spry.key_down "space" then
    local x, y = spry.mouse_pos()
    x, y = screen_to_tile(x, y)
    neighbors = map:neighbors_for_tile(x, y)
  end

  world:update(dt)

  spry.push_matrix()
  spry.scale(scale, scale)
    map:draw()

    if neighbors ~= nil then
      for k, v in ipairs(neighbors) do
        local x = v.x * tile + 4
        local y = v.y * tile + 4
        local w = tile / 4
        spry.draw_line_rect(x, y, w, w)
      end
    end

    if path ~= nil then
      for k, v in ipairs(path) do
        local x = v.x * tile
        local y = v.y * tile
        local w = tile

        spry.push_color(255, 0, k * 255 / #path, 64)
        spry.draw_filled_rect(x, y, w, w)
        spry.pop_color()
      end
    end

    world:draw()
  spry.pop_matrix()

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end
