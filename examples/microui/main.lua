function spry.conf(t)
  t.swap_interval = 0
  t.target_fps = 0
end

function spry.start()
  font = spry.default_font()
  mu = spry.microui

  log_input = mu.ref ""

  logbuf = mu.ref ""
  logbuf_updated = false

  bg = {
    r = mu.ref(90),
    g = mu.ref(95),
    b = mu.ref(100),
  }
  checks = map({true, false, true}, mu.ref)

  local function make_color(tab)
    return {
      r = mu.ref(tab[1]),
      g = mu.ref(tab[2]),
      b = mu.ref(tab[3]),
      a = mu.ref(tab[4]),
    }
  end

  colors = {
    {"text:",        mu.COLOR_TEXT,        make_color {230, 230, 230, 255}},
    {"border:",      mu.COLOR_BORDER,      make_color {25,  25,  25,  255}},
    {"windowbg:",    mu.COLOR_WINDOWBG,    make_color {50,  50,  50,  255}},
    {"titlebg:",     mu.COLOR_TITLEBG,     make_color {25,  25,  25,  255}},
    {"titletext:",   mu.COLOR_TITLETEXT,   make_color {240, 240, 240, 255}},
    {"panelbg:",     mu.COLOR_PANELBG,     make_color {0,   0,   0,   0}},
    {"button:",      mu.COLOR_BUTTON,      make_color {75,  75,  75,  255}},
    {"buttonhover:", mu.COLOR_BUTTONHOVER, make_color {95,  95,  95,  255}},
    {"buttonfocus:", mu.COLOR_BUTTONFOCUS, make_color {115, 115, 115, 255}},
    {"base:",        mu.COLOR_BASE,        make_color {30,  30,  30,  255}},
    {"basehover:",   mu.COLOR_BASEHOVER,   make_color {35,  35,  35,  255}},
    {"basefocus:",   mu.COLOR_BASEFOCUS,   make_color {40,  40,  40,  255}},
    {"scrollbase:",  mu.COLOR_SCROLLBASE,  make_color {43,  43,  43,  255}},
    {"scrollthumb:", mu.COLOR_SCROLLTHUMB, make_color {30,  30,  30,  255}},
  }
end

function write_log(str)
  local log = logbuf:get()
  if #log ~= 0 then
    log = log .. "\n"
  end
  log = log .. str
  logbuf:set(log)
  logbuf_updated = true
end

function test_window()
  -- do window
  if mu.begin_window("Demo Window", mu.rect(40, 40, 300, 450)) then
    local win = mu.get_current_container()
    local rect = win:rect()
    win:set_rect {
      x = rect.x,
      y = rect.y,
      w = math.max(rect.w, 240),
      h = math.max(rect.h, 300),
    }

    -- window info
    if mu.header "Window Info" then
      local win = mu.get_current_container()
      local rect = win:rect()
      mu.layout_row({54, -1}, 0)
      mu.label "Position:"
      mu.label(("%d, %d"):format(rect.x, rect.y))
      mu.label "Size:"
      mu.label(("%d, %d"):format(rect.w, rect.h))
    end

    -- labels + buttons
    if mu.header("Test Buttons", mu.OPT_EXPANDED) then
      mu.layout_row({86, -110, -1}, 0)
      mu.label "Test buttons 1:"
      if mu.button "Button 1" then write_log "Pressed button 1" end
      if mu.button "Button 2" then write_log "Pressed button 2" end
      mu.label "Test buttons 2:"
      if mu.button "Button 3" then write_log "Pressed button 3" end
      if mu.button "Popup" then mu.open_popup "Test Popup" end
      if mu.begin_popup "Test Popup" then
        mu.button "Hello"
        mu.button "World"
        mu.end_popup()
      end
    end

    -- tree
    if mu.header("Tree and Text", mu.OPT_EXPANDED) then
      mu.layout_row({140, -1}, 0)
      mu.layout_begin_column()
      if mu.begin_treenode "Test 1" then
        if mu.begin_treenode "Test 1a" then
          mu.label "Hello"
          mu.label "World"
          mu.end_treenode()
        end
        if mu.begin_treenode "Test 1b" then
          if mu.button "Button 1" then write_log "Pressed button 1" end
          if mu.button "Button 2" then write_log "Pressed button 2" end
          mu.end_treenode()
        end
        mu.end_treenode()
      end
      if mu.begin_treenode "Test 2" then
        mu.layout_row({54, 54}, 0)
        if mu.button "Button 3" then write_log "Pressed button 3" end
        if mu.button "Button 4" then write_log "Pressed button 4" end
        if mu.button "Button 5" then write_log "Pressed button 5" end
        if mu.button "Button 6" then write_log "Pressed button 6" end
        mu.end_treenode()
      end
      if mu.begin_treenode "Test 3" then
        mu.checkbox("Checkbox 1", checks[1])
        mu.checkbox("Checkbox 2", checks[2])
        mu.checkbox("Checkbox 3", checks[3])
        mu.end_treenode()
      end
      mu.layout_end_column()

      mu.layout_begin_column()
      mu.layout_row({-1}, 0)
      mu.text("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus ipsum, eu varius magna felis a nulla.")
      mu.layout_end_column()
    end

    -- background color sliders
    if mu.header("Background Color", mu.OPT_EXPANDED) then
      mu.layout_row({-78, -1}, 74)
      mu.layout_begin_column()
      mu.layout_row({46, -1}, 0)
      mu.label "Red:"; mu.slider(bg.r, 0, 255)
      mu.label "Green:"; mu.slider(bg.g, 0, 255)
      mu.label "Blue:"; mu.slider(bg.b, 0, 255)
      mu.layout_end_column()

      local r = mu.layout_next()
      local col = {
        r = math.floor(bg.r:get()),
        g = math.floor(bg.g:get()),
        b = math.floor(bg.b:get()),
        a = 255,
      }
      spry.clear_color(col.r, col.g, col.b, col.a)
      mu.draw_rect(r, col)
      local str = ("#%02X%02X%02X"):format(col.r, col.g, col.b)
      mu.draw_control_text(str, r, mu.COLOR_TEXT, mu.OPT_ALIGNCENTER)
    end

    mu.end_window()
  end
end

function log_window()
  if mu.begin_window("Log Window", mu.rect(350, 40, 300, 200)) then
    -- output text panel
    mu.layout_row({-1}, -25)
    mu.begin_panel "Log Output"
    local panel = mu.get_current_container()
    mu.layout_row({-1}, -1)
    mu.text(logbuf:get())
    mu.end_panel()
    if logbuf_updated then
      local sx, sy = panel:scroll()
      local x, y = panel:content_size()
      panel:set_scroll(sx, y)
      logbuf_updated = false
    end

    -- input textbox + buttons
    local submitted = false
    mu.layout_row({-120, -60, -1}, 0)
    if (mu.textbox(log_input) & mu.RES_SUBMIT) ~= 0 then
      mu.set_focus(mu.get_last_id())
      submitted = true
    end
    if mu.button "Submit" then submitted = true end
    if mu.button "Clear" then
      logbuf:set ""
    end
    if submitted then
      write_log(log_input:get())
      log_input:set ""
    end

    mu.end_window()
  end
end

function uint8_slider(label, palette, key)
  mu.push_id(label .. key)
  mu.slider(palette[key], 0, 255, 0, "%.0f")
  mu.pop_id()
end

function style_window()
  if mu.begin_window("Style Editor", mu.rect(350, 250, 300, 240)) then
    local style = mu.get_style()

    local sw = mu.get_current_container():body().w * 0.14
    mu.layout_row({80, sw, sw, sw, sw, -1}, 0)
    for _, color in ipairs(colors) do
      local label, idx, palette = color[1], color[2], color[3]
      mu.label(label)
      uint8_slider(label, palette, "r")
      uint8_slider(label, palette, "g")
      uint8_slider(label, palette, "b")
      uint8_slider(label, palette, "a")
      local c = {
        r = palette.r:get(),
        g = palette.g:get(),
        b = palette.b:get(),
        a = palette.a:get(),
      }
      mu.draw_rect(mu.layout_next(), c)
      style:set_color(idx, c)
    end
    mu.end_window()
  end
end

function spry.frame(dt)
  test_window()
  log_window()
  style_window()

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end