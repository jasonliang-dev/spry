local socket = require "socket"

function spry.conf(t)
  t.window_title = "Client"
end

function spry.start(arg)
  udp = socket.udp()
  udp:settimeout(0)
  udp:setpeername("localhost", 4242)

  my_id = os.time()
  entity = {
    x = 100,
    y = 100,
  }

  state = {}

  sendf("entity %d %d", entity.x, entity.y)

  font = spry.default_font()
end

send_update = create_thread(function()
  while true do
    sendf("entity %d %d", entity.x, entity.y)
    sendf "ping me"
    sleep(0.1)
  end
end)

recv_update = create_thread(function()
  while true do
    local data = udp:receive()
    if data ~= nil then
      local head, tail = data:match "(.-) (.*)"
      if head == "state" then
        local fn, err = load("return " .. tail)
        if not fn then
          error(err, 2)
        else
          state = fn()
        end
      end
    else
      yield()
    end
  end
end)

function spry.frame(dt)
  if spry.platform() ~= "html5" and spry.key_down "esc" then
    spry.quit()
  end

  entity.x, entity.y = spry.mouse_pos()

  resume(send_update)
  resume(recv_update)

  font:draw(stringify(state), 40, 40, 18)
  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end

function sendf(fmt, ...)
  local payload = ("%d " .. fmt):format(my_id, ...)
  udp:send(payload)
end
