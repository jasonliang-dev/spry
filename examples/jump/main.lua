function spry.conf(t)
  t.swap_interval = 1
  t.window_width = 540
  t.window_height = 970
  t.window_title = "Jump Game"
end

function spry.start()
  font = spry.default_font()
  atlas = spry.atlas_load "atlas.rtpa"

  audio_music = spry.audio_load "king-around-here.ogg"
  audio_beep = spry.audio_load "beep.ogg"
  audio_sparkle = spry.audio_load "sparkle.ogg"

  jump:reset()

  spry.clear_color(252, 223, 205, 255)

  local music = audio_music:make_sound()
  music:set_loop(true)
  music:set_vol(0.25)
  music:start()

  muted = spry.platform() == "html5"
end

function spry.frame(dt)
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  if spry.key_press "m" then
    muted = not muted
  end

  if muted then
    spry.set_master_volume(0)
  else
    spry.set_master_volume(1)
  end


  if game_over and spry.key_release "space" then
    jump:reset()
  end

  jump:update()

  b2:step(dt)
  world:update(dt)

  local target_y = max_height

  local blend = 1 - 0.85 ^ (dt * 20)
  camera.y = lerp(camera.y, target_y, blend)

  camera:begin_draw()
    world:draw()
  camera:end_draw()

  spry.push_color(0, 0, 0, 255)
    if spry.platform() ~= "html5" then
      font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
    end

    local text_size = 80
    local text = ("%.0f"):format(-max_height)
    local x = (spry.window_width() - font:width(text, text_size)) / 2
    local y = 100
    font:draw(text, x, y, text_size)

    if game_over then
      text_size = 60
      text = "Game Over"
      x = (spry.window_width() - font:width(text, text_size)) / 2
      y = (spry.window_height() - text_size) / 2 - 50
      font:draw(text, x, y, text_size)

      text_size = 30
      text = "Space to restart"
      x = (spry.window_width() - font:width(text, text_size)) / 2
      y = (spry.window_height() - text_size) / 2
      font:draw(text, x, y, text_size)
    end
  spry.pop_color()
end
