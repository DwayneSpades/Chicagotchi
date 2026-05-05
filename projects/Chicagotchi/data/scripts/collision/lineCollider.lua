lineCollider = {}
lineCollider.__index = lineCollider

--may want to consider giving each of my personal data types an enum/string to verify their input
function lineCollider.new(obj1,obj2)
  local instance = setmetatable({}, lineCollider)
  instance.point1 = obj1
  instance.point2 = obj2

  return instance
end

function lineCollider:draw(colorNum)
  local color = colorNum or 1
  --insert myrtle circle draw command
  myrtle.drawLine(self.point1.x,self.point1.y,self.point2.x,self.point2.y,"0x"..myPallet[color])
end


function lineCollider.length(l1)
  return vector2.magnitude(l1.point2 - l1.point1)
end

function lineCollider.lengthSq(l1)
  return vector2.magnitudeSq(l1.point2 - l1.point1)
end




