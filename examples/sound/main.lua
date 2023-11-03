function spry.start()
  font = spry.default_font()
  audio = spry.audio_load "hit01.ogg"
end

function spry.frame(dt)
  if spry.key_press "space" then
    local sound = audio:make_sound()
    sound:start()
  end

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end
