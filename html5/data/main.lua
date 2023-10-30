function spry.start()
  font = spry.default_font()
end

function spry.frame(dt)
  font:draw("Hello, World!", 100, 100, 36)
end