function spry.start()
  jp = spry.font_load("noto-sans-jp.ttf")
  font = spry.default_font()

  -- other files implicitly loaded before main.lua
  player = Player(200, 200)
end

function spry.frame(dt)
  player:update(dt)
  player:draw()

  jp:draw("Hello, 世界!", 100, 100, 24)
  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end