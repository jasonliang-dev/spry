require "animal"
class("Crab", Animal)

function Crab:new(x, y)
  self.super.new(self, x, y)
  self.kind = "Crab"
  self.legs = 8
end
