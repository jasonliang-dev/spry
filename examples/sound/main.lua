function spry.start()
  font = spry.default_font()
  audio_hit = spry.audio_load "hit01.ogg"
  audio_bgm = spry.audio_load "bgm.ogg"
  audio_engine = spry.audio_load "engine.ogg"

  music = audio_bgm:make_sound()
  music:set_loop(true)

  engine = audio_engine:make_sound()
  engine:set_loop(true)
  engine:set_vol(0.1)
  engine:start()

  music_playing = false
end

local function play_at(vol)
  local s = audio_hit:make_sound()
  s:set_vol(vol)
  s:start()
end

function spry.frame(dt)
  if spry.key_press "m" then
    music_playing = not music_playing
    if music_playing then
      music:start()
    else
      music:stop()
    end
  end

  if spry.key_press "-" then
    spry.set_master_volume(0.1)
  end

  if spry.key_press "=" then
    spry.set_master_volume(1)
  end

  local vol = music:vol()
  if spry.key_down "j" then vol = vol - 0.01 end
  if spry.key_down "k" then vol = vol + 0.01 end
  vol = clamp(vol, 0, 1)
  music:set_vol(vol)

  font:draw(("music volume: %.2f"):format(vol), 100, 100, 24)

  if spry.mouse_click(0) then engine:set_vol(0.8) end
  if spry.mouse_release(0) then engine:set_vol(0.1) end

  local x, y = spry.mouse_pos()
  x = lerp(-0.5, 0.5, x / spry.window_width())
  y = lerp(-0.5, 0.5, y / spry.window_height())

  local dx, dy = spry.mouse_delta()

  engine:set_pos(x, y)
  engine:set_dir(dx, dy)
  engine:set_vel(dx, dy)

  font:draw(("engine pos: %.2f, %.2f"):format(x, y), 100, 120, 24)
  font:draw(("engine vel: %.2f, %.2f"):format(dx, dy), 100, 140, 24)

  if spry.key_press "1" then play_at(0.1) end
  if spry.key_press "2" then play_at(0.2) end
  if spry.key_press "3" then play_at(0.3) end
  if spry.key_press "4" then play_at(0.4) end
  if spry.key_press "5" then play_at(0.5) end
  if spry.key_press "6" then play_at(0.6) end
  if spry.key_press "7" then play_at(0.7) end
  if spry.key_press "8" then play_at(0.8) end
  if spry.key_press "9" then play_at(0.9) end
  if spry.key_press "0" then play_at(1.0) end

  if spry.key_press "left" then
    local s = audio_hit:make_sound()
    s:set_pan(-0.5)
    s:start()
  end

  if spry.key_press "right" then
    local s = audio_hit:make_sound()
    s:set_pan(0.5)
    s:start()
  end

  if spry.key_press "up" then
    local s = audio_hit:make_sound()
    s:set_pitch(2)
    s:start()
  end

  if spry.key_press "down" then
    local s = audio_hit:make_sound()
    s:set_pitch(0.5)
    s:start()
  end

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end
