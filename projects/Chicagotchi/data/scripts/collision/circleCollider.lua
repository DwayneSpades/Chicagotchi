circleCollider={}
circleCollider.__index = circleCollider

function circleCollider.new(pos,r)
  local instance = setmetatable({}, circleCollider)
  
  instance.radius = r or 16
  instance.position = pos or vector2.new(0,0)
  instance.color = 1
  
  return instance
end

function circleCollider:draw(colorNum)
  local color = colorNum or 1
  --insert myrtle circle draw command
  myrtle.drawCircle(self.position.x,self.position.y,self.radius,"0x"..myPallet[color])
end

--point collision
--[[
function circleCollider:pointInCircle(p)
  d = distance(self.position,p)
  return d <= self.radius
end
]]


