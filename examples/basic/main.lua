function spry.start()
  jp = spry.font_load("noto-sans-jp.ttf")
  font = spry.default_font()
end

function spry.frame(dt)
  if spry.key_down "esc" then
    spry.quit()
  end

  jp:draw('Hello, 世界!', 100, 100, 24)
  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end