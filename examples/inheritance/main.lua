function spry.start()
  font = spry.default_font()

  a = Dog(100, 100)
  b = Crab(100, 120)
end

function spry.frame(dt)
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  a:say()
  b:say()
end
