function spry.conf(t)
  t.swap_interval = 1
  t.target_fps = 0
end

function spry.start()
  font = spry.default_font()
  map = spry.tilemap_load "map.ldtk"
  map:make_graph('IntGrid', { [1] = 1 })
  -- map:print_graph()
  map:astar(0, 0, 7, 2)
end

function spry.frame(dt)
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  spry.push_matrix()
  spry.scale(3, 3)
    map:draw()
  spry.pop_matrix()

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end
