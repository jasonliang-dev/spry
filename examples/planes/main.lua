function spry.conf(t)
  t.swap_interval = 0
  t.reload_interval = 0.001
  t.window_width = 1366
  t.window_height = 768
  t.window_title = "Plane Game"
end

function spry.start(arg)
  if arg[#arg] == "-mobdebug" then unsafe_require "mobdebug".start() end
  if os.getenv "LOCAL_LUA_DEBUGGER_VSCODE" == "1" then unsafe_require "lldebugger".start() end

  composite = spry.image_load "_composite.png"
  atlas = spry.atlas_load "atlas.rtpa"
  world = World()

  interval(2, function()
    world:add(Enemy())
  end)

  player = world:add(Player())
end

function spry.frame(dt)
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  world:update(dt)

  composite:draw(0, 0, 0, 3, 3)
  world:draw()
end
