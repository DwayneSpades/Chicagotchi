--has a:
--holds the collision data for game objects
--can visualize itself for debugging purpouses

AABBCollider = {}
AABBCollider.__index = AABBCollider

function AABBCollider.new(pos,width,height)
  local instance = setmetatable({}, AABBCollider)
  instance.position = pos or vector2.new(0,0)
  instance.width = width or 32
  instance.height = height or 32
  instance.collided = false
  
  return instance
end

function AABBCollider:draw(colorNum)
  local color = colorNum or 1
  --insert myrtle circle draw command
  myrtle.drawRectangle(self.position.x,self.position.y,self.width,self.height,"0x"..myPallet[color])
end

function AABBCollider:getMin()
  return self.position
end

function AABBCollider:getMax()
  return vector2.new(self.position.x+self.width,self.position.y+self.height )
end
--[[
function AABBCollider:pointInRectangle(p)
  local min = self:getMin()
  local max = self:getMax()
  
  return 
  min.x <= p.x and
  min.y <= p.y and 
  max.x >= p.x and 
  max.y >= p.y
  
end


]]

