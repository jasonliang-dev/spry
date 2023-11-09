function spry.conf(t)
  t.swap_interval = 1
  t.target_fps = 0
end

function spry.start()
  font = spry.default_font()

  map = spry.tilemap_load "map.ldtk"
  map:make_graph("IntGrid", { [1] = 1 }, 4)

  start = { x = 0, y = 0 }
  goal = { x = 0, y = 0 }

  world = World()

  scale = 2
  tile = 16

  spry.clear_color(155, 212, 195, 255)
end

function screen_to_world(x, y)
  x = x / scale
  y = y / scale

  return x, y
end

function spry.frame(dt)
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  if spry.mouse_down(0) then
    local x, y = spry.mouse_pos()
    start.x, start.y = screen_to_world(x, y)
    path = map:astar(start.x, start.y, goal.x, goal.y)
    if spry.mouse_click(0) and #path > 0 then
      world:add(Actor(path))
    end
  end

  if spry.mouse_down(1) then
    local x, y = spry.mouse_pos()
    goal.x, goal.y = screen_to_world(x, y)
    path = map:astar(goal.x, goal.y, start.x, start.y)
    if spry.mouse_click(1) and #path > 0 then
      world:add(Actor(path))
    end
  end

  world:update(dt)

  spry.push_matrix()
  spry.scale(scale, scale)
    map:draw()

    if path ~= nil then
      for i = 1, #path - 1 do
        local x0 = path[i + 0].x + tile / 2
        local y0 = path[i + 0].y + tile / 2
        local x1 = path[i + 1].x + tile / 2
        local y1 = path[i + 1].y + tile / 2

        spry.push_color(255, 0, i * 255 / #path, 255)
        spry.draw_line(x0, y0, x1, y1)
        spry.pop_color()
      end
    end

    world:draw()
  spry.pop_matrix()

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end
