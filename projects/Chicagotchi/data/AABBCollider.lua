--has a:
--holds the collision data for game objects
--can visualize itself for debugging purpouses

AABB_Collider = {}
AABB_Collider.__index = AABB_Collider

function AABB_Collider.new(pos,width,height)
  local instance = setmetatable({}, AABB_Collider)
  instance.position = pos or vector2.new(0,0)
  instance.width = width or 32
  instance.height = height or 32
  instance.collided = false
  
  return instance
end

function AABB_Collider:pointInRectangle(p)
  local min = self:getMin()
  local max = self:getMax()
  
  return 
  min.x <= p.x and
  min.y <= p.y and 
  max.x >= p.x and 
  max.y >= p.y
  
end

function AABB_Collider:getMin()
  return self.position
end

function AABB_Collider:getMax()
  return vector2.new(self.position.x+self.width,self.position.y+self.height )
end

--update collider position
function AABB_Collider:setPosition(pos)
  self.position = pos
end

function AABB_Collider:draw()
  --insert the arduino rectangle draw here
  --love.graphics.rectangle("line",self.position.x,self.position.y, self.width, self.height)
end
