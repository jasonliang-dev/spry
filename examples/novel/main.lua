function spry.conf(t)
  t.swap_interval = 1
  t.window_width = 960
  t.window_height = 540
end

function spry.start()
  font = spry.font_load "noto-sans-jp.ttf"

  samplers = {
    linear = spry.make_sampler {
      min_filter = "linear",
      mag_filter = "linear",
      mipmap_filter = "linear",
    },
    default = spry.default_sampler(),
  }

  stat = {
    dialog_time = 0,
    dialog = "",
  }

  preload = {}
  preload.ch = spry.make_channel "preload"
  preload.thread = spry.make_thread [[
    spry.image_load("bg/Train_Day.png", true)
    spry.image_load("bg/Street_Spring_Day.png", true)
    spry.image_load("bg/School_Hallway_Day.png", true)

    spry.image_load("char/Alice_Blush.png", true)
    spry.image_load("char/Alice_Default.png", true)
    spry.image_load("char/Alice_Happy.png", true)
    spry.image_load("char/Alice_Worried.png", true)
    spry.image_load("char/Alice_Embarrassed.png", true)

    local ch = spry.get_channel "preload"
    ch:send "done"
  ]]
end

local function menu_rect(right, top, i)
  local w = 300
  local h = 30
  local x = right - w
  local y = top - ((#stat.menu - i) * 40) - 35

  return x, y, w, h
end

local function progress(co, ...)
  local ok, err = coroutine.resume(co, ...)
  if not ok then
    error(err, 2)
  end

  if coroutine.status(co) == "dead" then
    spry.quit()
  end
end

local function frame(dt)
  local h = 150
  local top = spry.window_height() - h

  local w = math.min(800, spry.window_width())
  local left = (spry.window_width() - w) / 2

  stat.dialog_time = stat.dialog_time + dt
  local dialog = ""

  local len = utf8.len(stat.dialog)
  len = math.min(len, math.floor(stat.dialog_time * 100))
  len = utf8.offset(stat.dialog, len + 1)
  if len then
    dialog = stat.dialog:sub(0, len - 1)
  end

  if spry.mouse_click(0) then
    if #dialog ~= #stat.dialog then
      stat.dialog_time = 9999

    elseif stat.menu then
      for i = 1, #stat.menu do
        local x, y, w, h = menu_rect(left + w, top, i)
        if rect_overlap(x, y, w, h, spry.mouse_pos()) then
          stat.menu = nil
          progress(script, i)
          break
        end
      end

    else
      progress(script)
    end
  end

  samplers.linear:use()

  if stat.bg then
    local x = spry.window_width() / 2
    local y = spry.window_height() / 2
    local ox = stat.bg:width() / 2
    local oy = stat.bg:height() / 2
    local s = math.max(x / ox, y / oy)
    stat.bg:draw(x, y, 0, s, s, ox, oy)
  end

  if stat.char then
    local x = spry.window_width() / 2
    local y = spry.window_height()
    local ox = stat.char:width() / 2
    local oy = stat.char:height()
    local s = spry.window_height() / stat.char:height()
    stat.char:draw(x, y, 0, s, s, ox, oy)
  end

  samplers.default:use()

  -- dialog
  spry.push_color(0, 0, 0, 255 * 0.9)
    spry.draw_filled_rect(left, top, w, h)
  spry.pop_color()
  local p = 10
  local size = 24
  local wrap = w - (p * 2)
  font:draw(dialog, left + p, top + p, size, wrap)

  -- color
  if stat.color then
    spry.push_color(table.unpack(stat.color))
      spry.draw_filled_rect(left, top, w, 4)
    spry.pop_color()
  end

  -- name
  if stat.name then
    local h = 40
    spry.push_color(0, 0, 0, 255 * 0.9)
      spry.draw_filled_rect(left, top - h - 5, 200, h)
    spry.pop_color()
    local size = 30
    local x = left + 8
    local y = top - h - 7
    font:draw(stat.name, x, y, size)
  end

  -- menu
  if stat.menu then
    for k, v in ipairs(stat.menu) do
      local x, y, w, h = menu_rect(left + w, top, k)
      if rect_overlap(x, y, w, h, spry.mouse_pos()) then
        spry.push_color(128, 32, 32, 255 * 0.9)
      else
        spry.push_color(0, 0, 0, 255 * 0.9)
      end
      spry.draw_filled_rect(x, y, w, h)
      spry.pop_color()

      font:draw(v, x + 5, y - 3, 24)
    end
  end
end

function spry.frame(dt)
  if preload.thread ~= nil then
    font:draw("Loading...", (spry.elapsed() * 100) % spry.window_width(), spry.window_height() - 40, 28)

    if preload.ch:try_recv() == "done" then
      preload.thread:join()
      preload.thread = nil

      progress(script)
    end
  else
    frame(dt)
  end
end
