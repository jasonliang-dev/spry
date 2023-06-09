R"lua"--(

-- on startup, all lua files in the mounted directory are loaded. lua files can
-- return values (usually libraries), so those values are stored here.

spry.files = {}

-- define these in case user doesn't define them

function spry.conf() end

function spry.start() end

function spry.frame(dt)
  if spry.key_down "esc" then spry.quit() end

  if default_font == nil then
    default_font = spry.default_font()
  end

  local text = "no game!"
  local text_size = 36
  local x = (spry.window_width() - default_font:width(text, text_size)) / 2
  local y = (spry.window_height() - text_size) * 0.45

  if default_font == nil then
    default_font = spry.default_font()
  end
  default_font:draw(text, x, y, text_size)
end

-- object oriented

local class_mt = {}
function class_mt:__call(...)
  local obj = setmetatable({}, self)
  obj:new(...)
  return obj
end

function class(name)
  local obj = {}
  obj.__index = obj
  rawset(_G, name, setmetatable(obj, class_mt))
end

-- 2d vector

class "vec2"

function vec2:new(x, y)
  self.x = x
  self.y = y
end

function vec2:normalize()
  local x = self.x
  local y = self.y

  local len = math.sqrt(x * x + y * y)
  if len == 0 then
    return vec2(0, 0)
  end

  return vec2(x / len, y / len)
end

function vec2:distance(rhs)
  local dx = rhs.x - self.x
  local dy = rhs.y - self.y
  return math.sqrt(dx * dx + dy * dy)
end

function vec2:direction(rhs)
  local dx = rhs.x - self.x
  local dy = rhs.y - self.y
  return math.atan(dy, dx)
end

function vec2:lerp(rhs, t)
  local x = self.x
  local y = self.y
  return vec2(x + (rhs.x - x) * t, y + (rhs.y - y) * t)
end

function vec2:dot(rhs)
  return self.x * rhs.x + self.y * rhs.y
end

function vec2:unpack()
  return self.x, self.y
end

function vec2.__add(lhs, rhs)
  return vec2(lhs.x + rhs.x, lhs.y + rhs.y)
end

function vec2.__sub(lhs, rhs)
  return vec2(lhs.x - rhs.x, lhs.y - rhs.y)
end

function vec2.__mul(lhs, rhs)
  return vec2(lhs.x * rhs.x, lhs.y * rhs.y)
end

function vec2.__div(lhs, rhs)
  return vec2(lhs.x / rhs.x, lhs.y / rhs.y)
end

function vec2:__len()
  local x = self.x
  local y = self.y
  return math.sqrt(x * x + y * y)
end

function vec2.__eq(lhs, rhs)
  return lhs.x == rhs.x and lhs.y == rhs.y
end

function vec2:__tostring()
  return "vec2(" .. self.x .. ", " .. self.y .. ")"
end

-- group of entities in a world

class "World"

local function _cell_pos(x, y)
  return math.floor(x / 64), math.floor(y / 64)
end

local function _cell_key(x, y)
  return x + y * 1e7
end

function World:new()
  self.next_id = 0
  self.by_id = {}
  self.by_mt = {}
  self.by_cell = {}
  self.to_create = {}
  self.to_kill = {}
end

function World:add(obj)
  obj.id = self.next_id

  if obj.z_index == nil then
    obj.z_index = 0
  end

  self.next_id = self.next_id + 1
  self.to_create[obj.id] = obj
  return obj
end

function World:kill(id)
  local obj
  if type(id) == "table" then
    obj = id
  else
    obj = self.by_id[id]
  end

  if obj ~= nil then
    self.to_kill[obj.id] = obj
  end
end

function World:update(dt)
  for id, obj in pairs(self.to_create) do
    local mt = getmetatable(obj)
    if self.by_mt[mt] == nil then
      self.by_mt[mt] = {}
    end

    self.by_mt[mt][id] = obj
    self.by_id[id] = obj
    self.to_create[id] = nil
  end

  --[[
  for cell, arr in pairs(self.by_cell) do
    for id in pairs(arr) do
      arr[id] = nil
    end
  end

  for id, obj in pairs(self.by_id) do
    if obj.x and obj.y then
      local cell = _cell_key(_cell_pos(obj.x, obj.y))
      if self.by_cell[cell] == nil then
        self.by_cell[cell] = {}
      end

      self.by_cell[cell][id] = obj
    end
  end
  ]]

  for id, obj in pairs(self.by_id) do
    obj:update(dt)
  end

  for id, obj in pairs(self.to_kill) do
    if obj.on_death then
      obj:on_death()
    end

    local mt = getmetatable(obj)
    self.by_mt[mt][id] = nil
    self.by_id[id] = nil
    self.to_kill[id] = nil
  end
end

function World:draw()
  local sorted = {}
  for id, obj in pairs(self.by_id) do
    sorted[#sorted + 1] = obj
  end

  table.sort(sorted, function (lhs, rhs)
    return lhs.z_index < rhs.z_index
  end)

  for k, obj in ipairs(sorted) do
    obj:draw()
  end
end

function World:query_id(id)
  return self.by_id[id]
end

function World:query_mt(mt)
  if self.by_mt[mt] == nil then
    self.by_mt[mt] = {}
  end

  return self.by_mt[mt]
end

function World:query_near(x, y, mt)
  local nearby = {}
  x, y = _cell_pos(x, y)
  for xx = x - 1, x + 1 do
    for yy = y - 1, y + 1 do
      local cell = _cell_key(xx, yy)
      if self.by_cell[cell] == nil then
        self.by_cell[cell] = {}
      end

      if mt == nil then
        for id, obj in pairs(self.by_cell[cell]) do
          nearby[id] = obj
        end
      else
        for id, obj in pairs(self.by_cell[cell]) do
          if getmetatable(obj) == mt then
            nearby[id] = obj
          end
        end
      end
    end
  end

  return nearby
end

-- bouncing spring

class "Spring"

function Spring:new(k, d)
  self.stiffness = k or 400
  self.damping = d or 28
  self.x = 0
  self.v = 0
end

function Spring:update(dt)
  local a = -self.stiffness * self.x - self.damping * self.v
  self.v = self.v + a * dt
  self.x = self.x + self.v * dt
end

function Spring:pull(f)
  self.x = self.x + f
end

-- intervals/timeouts

local timer = {}
spry.timer = timer

timer.next_id = 0
timer.intervals = {}
timer.timeouts = {}

function spry.timer_update(dt)
  for id, t in pairs(timer.intervals) do
    t.elapsed = t.elapsed + dt
    if t.elapsed >= t.seconds then
      t.elapsed = t.elapsed - t.seconds
      t.callback()
    end
  end

  for id, t in pairs(timer.timeouts) do
    t.elapsed = t.elapsed + dt
    if t.elapsed >= t.seconds then
      t.elapsed = t.elapsed - t.seconds
      t.callback()
      timer.timeouts[id] = nil
    end
  end
end

function interval(sec, action)
  local id = timer.next_id

  timer.intervals[id] = {
    callback = action,
    seconds = sec,
    elapsed = 0,
  }
  timer.next_id = timer.next_id + 1

  return id
end

function timeout(sec, action)
  local id = timer.next_id

  timer.timeouts[id] = {
    callback = action,
    seconds = sec,
    elapsed = 0,
  }
  timer.next_id = timer.next_id + 1

  return id
end

function stop_interval(id)
  if timer.intervals[id] ~= nil then
    timer.intervals[id] = nil
  end
end

function stop_timeout(id)
  if timer.timeouts[id] ~= nil then
    timer.timeouts[id] = nil
  end
end

-- utility functions

local function _stringify(value, visited, indent)
  if type(value) == "table" then
    local mt = getmetatable(value)
    if mt ~= nil and rawget(mt, "__tostring") then
      return tostring(value)
    elseif visited[value] then
      return tostring(value)
    end

    visited[value] = true
    local white = string.rep(" ", indent + 2)
    local s = "{\n"
    for k,v in pairs(value) do
      s = s .. white .. tostring(k) .. " = " .. _stringify(v, visited, indent + 2) .. ",\n"
    end

    return s .. string.rep(" ", indent) .. "}"
  else
    return tostring(value)
  end
end

function stringify(value)
  return _stringify(value, {}, 0)
end

function clamp(n, min, max)
  if n < min then return min end
  if n > max then return max end
  return n
end

function sign(x)
  if x < 0 then return -1 end
  if x > 0 then return 1 end
  return x
end

function lerp(src, dst, t)
  return src + (dst - src) * t
end

function direction(x0, y0, x1, y1)
  local dx = x1 - x0
  local dy = y1 - y0
  return math.atan(dy, dx)
end

function heading(angle, mag)
  local x = math.cos(angle) * mag
  local y = math.sin(angle) * mag
  return x, y
end

function delta_angle(src, dst)
  local tau = math.pi * 2
  return (dst - src + math.pi) % tau - math.pi
end

function distance(x0, y0, x1, y1)
  local dx = x1 - x0
  local dy = y1 - y0
  return math.sqrt(dx * dx + dy * dy)
end

function normalize(x, y)
  local len = math.sqrt(x * x + y * y)
  if len == 0 then
    return 0, 0
  end

  return x / len, y / len
end

function dot(x0, y0, x1, y1)
  return x0 * x1 + y0 * y1
end

function random(min, max)
  return math.random() * (max - min) + min
end

function clone(t)
  local tab = {}
  for k, v in pairs(t) do
    tab[k] = v
  end
  return tab
end

function push(arr, x)
  arr[#arr + 1] = x
end

function map(arr, fn)
  local t = {}
  for k, v in ipairs(arr) do
    t[k] = fn(v)
  end
  return t
end

function filter(arr, fn)
  local t = {}
  for k, v in ipairs(arr) do
    if fn(v) then
      t[#t + 1] = v
    end
  end
  return t
end

function zip(lhs, rhs)
  local len = math.min(#lhs, #rhs)
  local t = {}
  for i = 1, len do
    t[i] = {lhs[i], rhs[i]}
  end
  return t
end

function choose(arr)
  return arr[math.random(#arr)]
end

function sortpairs(t)
  local keys = {}
  for k in pairs(t) do
    keys[#keys + 1] = k
  end
  table.sort(keys)

  local i = 0
  return function()
    i = i + 1
    local k = keys[i]
    return k, t[k]
  end
end

function create_thread(fn)
  return coroutine.create(fn)
end

function resume(co, ...)
  local ok, err = coroutine.resume(co, ...)
  if not ok then
    error(err, 2)
  end
end

function yield(...)
  return select(2, coroutine.yield(...))
end

function sleep_for(secs, dt)
  while secs > 0 do
    secs = secs - dt
    dt = yield()
  end

  return dt
end

function import(name)
  return table.unpack(spry.files[name])
end

--)lua"--"