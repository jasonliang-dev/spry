function spry.conf(t)
  t.swap_interval = 0
  t.target_fps = 500
end

function spry.start()
  jp = spry.font_load("noto-sans-jp.ttf")
  font = spry.default_font()

  world = World()
  world:add(Player(200, 200))
end

function spry.frame(dt)
  world:update(dt)
  world:draw()

  if spry.mouse_down(0) then
    world:add(Player(spry.mouse_pos()))
  end

  jp:draw("Hello, 世界!", 100, 100, 24)
  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end