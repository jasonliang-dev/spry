function spry.conf(t)
  t.window_width = 1366
  t.window_height = 768
  t.window_title = "Plane Game"
  t.swap_interval = 0
end

function spry.start()
  composite = spry.image_load "_composite.png"
  atlas = spry.atlas_load "atlas.rtpa"
  world = World()

  interval(2, function()
    world:add(Enemy())
  end)

  player = world:add(Player())
end

function spry.frame(dt)
  if spry.key_down "esc" then
    spry.quit()
  end

  world:update(dt)

  composite:draw(0, 0, 0, 3, 3)
  world:draw()
end
