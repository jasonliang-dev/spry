require "animal"
class("Dog", Animal)

function Dog:new(x, y)
  self.super.new(self, x, y)
  self.kind = "Dog"
  self.legs = 4
end
