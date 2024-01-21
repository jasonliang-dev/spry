local function bg(name)
  if name then
    stat.bg = spry.image_load("bg/" .. name .. ".png", true)
  else
    stat.bg = nil
  end
end

local function ch(name)
  if name then
    stat.char = spry.image_load("char/" .. name .. ".png", true)
  else
    stat.char = nil
  end
end

function menu(tab)
  stat.menu_time = 0
  stat.menu = tab
  if #tab > 0 then
    return coroutine.yield()
  end
end

local function name(str)
  stat.name = str
end

local function color(r, g, b)
  if r then
    stat.color = {r, g, b, 255}
  else
    stat.color = nil
  end
end

local function say(str)
  stat.dialog_time = 0
  stat.dialog = str
  coroutine.yield()
end

local function _(str)
  name(nil)
  color(nil)
  say(str)
end

local function p(str)
  name "Player"
  color(255, 255, 255)
  say(str)
end

local function a(str)
  name "Alice"
  color(255, 96, 96)
  say(str)
end

script = coroutine.create(function()
  bg "Train_Day"
  _ "You're on the train on the way to school."
  _ "You hear the announcer through the train speakers."
  _ "\"The next station is, Aoyama-itchōme. Aoyama-itchōme.\""
  _ "This is your stop. You leave the train."

  bg "School_Hallway_Day"
  _ "You arrived to school on time. You see one of your classmates walk towards you."

  ch "Alice_Blush"
  a "Hey! How was your weekend?"

  local i = menu {
    "It was fun!",
    "It was boring.",
  }
  ch "Alice_Default"

  if i == 1 then
    p "My weekend was great. I had a lot of fun."

    ch "Alice_Happy"
    a "I'm glad to hear it."

  elseif i == 2 then
    p "The weekend was pretty dull."

    ch "Alice_Worried"
    a "Aww. Well, maybe next week will be better."
  end

  _ "The school bell rings."

  ch "Alice_Default"
  a "Ah, I guess we'll talk later."

  ch "Alice_Embarrassed"
  a "See you!"

  ch()
  _ "Alice leaves."
  _ "You head to your classroom."
end)
