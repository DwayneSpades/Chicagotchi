enemy = {}
enemy.__index = enemy

function enemy.new(pos)
  local instance = setmetatable({}, enemy)
  instance.position = pos or vector2.new((240/2)-32,(135/2)-32) 
  instance.dir = vector2.new(1,-1)
  
  instance.collider = circleCollider.new(instance.position, 16)
  
  instance.xp = points or 0
  instance.nextLevel = 10
  instance.level = level or 1
  
  instance.health = 10
  instance.speed = 2
  instance.strength = 3
  
  instance.sprite = groofSprite
  
  return instance
end

function enemy:update()
  if self.speed > 2 then
    self.speed = self.speed - 0.005
  end
  
  self.position = self.position + (vector2.normalize(self.dir) * self.speed)
  self.collider.position = self.position
end

function enemy:draw()
  self.sprite:draw(self.position-vector2.new(32,32))
  self.collider:draw(5)
end
