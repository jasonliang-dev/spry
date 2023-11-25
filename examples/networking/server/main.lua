-- this server *should* work with a lua interpreter with luasockets, but you
-- can also run this server with the spry binary

local socket = require "socket"

local udp = socket.udp()
udp:settimeout(0)
udp:setsockname('*', 4242)
print "listening on port 4242"

local state = {}
local connections = {}

local cmd = {}

function cmd.entity(id, data)
  local x, y = data:match "(.-) (.*)"
  state[id] = { x = x, y = y }
end

function cmd.ping(id, data, ip, port)
  local tab = {}
  for k, v in pairs(state) do
    tab[#tab + 1] = ("[%s]={x=%s,y=%s}"):format(k, v.x, v.y)
  end
  local payload = "state {" .. table.concat(tab, ",") .. "}"
  udp:sendto(payload, ip, port)
end

while true do
  while true do
    local data, ip, port = udp:receivefrom()
    if data == nil then
      break
    end

    local id, head, tail = data:match "(.-) (.-) (.*)"
    connections[id] = os.time()

    local fn = cmd[head]
    if fn ~= nil then
      fn(id, tail, ip, port)
    end
  end

  -- remove connections after 3 seconds
  local now = os.time()
  local kill = {}
  for k, v in pairs(connections) do
    if os.difftime(now, v) >= 3 then
      kill[#kill + 1] = k
    end
  end

  for _, id in ipairs(kill) do
    connections[id] = nil
    state[id] = nil
  end

  socket.sleep(0.01)
end

-- quit before we make a window
os.exit()