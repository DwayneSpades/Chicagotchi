myPet = {}
myPet.__index = myPet

function myPet.new(sprite)
  local instance = setmetatable({}, myPet)
  instance.position = vector2.new(240/2,135/2) 
  instance.dir = vector2.new(-1,-1)
  
  instance.collider = circleCollider.new(instance.position, 16)
  
  
  instance.xp = points or 0
  instance.nextLevel = 10
  instance.level = level or 1
  
  instance.health = 10
  instance.speed = 2
  instance.strength = 3
  
  instance.sprite = sprite or gatoSprite
  
  return instance
end

function myPet:update()
  
  if self.speed > 2 then
    self.speed = self.speed - 0.005
  end
  
  self.position = self.position + (vector2.normalize(self.dir) * self.speed)
  self.collider.position = self.position
end

function myPet:draw()
  self.sprite:draw(self.position-vector2.new(32,32))
  self.collider:draw(3)
end
