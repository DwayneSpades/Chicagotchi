
vector2 ={}
vector2.__index = vector2

function vector2.new(x,y,z)
  local instance = setmetatable({}, vector2)
  instance.name="vector2"
  instance.x = x or 0
  instance.y = y or 0
  
  --only exists for special computations
  instance.z = z or 0
  instance.w=0
  
  return instance
end

function vector2:print()
  return ("X: "..self.x.." Y: "..self.y)
end
---override the meta methods for vector2 class
--***CAUTION*** I'm a little worried about this override style for arithmentics will slow down the game because of creating a new vector potentially every frame

vector2.__eq = function(v1,v2)
  return (v1.x == v2.x and v1.y == v2.y)
end

vector2.__add = function(v1,v2)
  local v = vector2.new()
  if(type(v2) == "number") then
    v.x = v1.x + v2
    v.y = v1.y + v2
  else
    v.x = v1.x + v2.x
    v.y = v1.y + v2.y
    
  end
  
  return v
end

vector2.__sub = function(v1,v2)
  local v = vector2.new()
  
  if(type(v2) == "number") then
    v.x = v1.x - v2
    v.y = v1.y - v2
  else
    v.x = v1.x - v2.x
    v.y = v1.y - v2.y
    
  end
  
  return v
end

--the operators with mismatched types don't go both ways so I should add that cuntionality at some point to make them commutative
vector2.__mul = function(v1,v2)
  local v = vector2.new()
  --allows me to handle multiple data types
  if(type(v2) == "number") then
    v.x = v1.x * v2
    v.y = v1.y * v2
  else
    v.x = v1.x * v2.x
    v.y = v1.y * v2.y
    
  end
  
  return v
end

--the divide for 2 tables
vector2.__div = function(v1,v2)
  local v = vector2.new()
  
  --allows me to handle multiple data types
  if(type(v2) == "number") then
    v.x = v1.x / v2
    v.y = v1.y / v2
  else
    v.x = v1.x / v2.x
    v.y = v1.y / v2.y
    
  end
  
  return v
end

function vector2.dot(v1,v2)
  return (v1.x * v2.x) + (v1.y * v2.y)
end

function vector2.cross(v1,v2)
	return (v1.x*v2.y) - (v1.y*v2.x)
end

function vector2.crossWinding(v1,v2)
  return vector3.new(((v1.y * v2.z) - (v1.z*v2.y)), -((v1.x*v2.z) - (v1.z*v2.x)), ((v1.x*v2.y) - (v1.y*v2.x)))
end

function vector2.magnitude(v1)
  return math.sqrt(vector2.dot(v1,v1))
end

function vector2:magnitude()
  return math.sqrt(vector2.dot(self,self))
end

function vector2.magnitudeSq(v1)
  return vector2.dot(v1,v1)
end

function vector2.normalized(v1)
  local mag = vector2.magnitude(v1)
  if(mag==0)then
    print("DIVIDE BY ZERO")
    mag=1
  end
  
  local v = v1 * (1 / mag)
  return v
end

function vector2:normalize()
  
  local v = self * (1 / vector2.magnitude(self))
  return v
end

function vector2:round()
  return vector2.new(roundNumber(self.x),roundNumber(self.y))
end

function vector2.angle(v1,v2)
  --!!!!returns in radians!!!!
  local m = math.sqrt(vector2.magnitudeSq(v1) * vector2.magnitudeSq(v2))
  return math.acos(vector2.dot(v1,v2) / m)
end

function vector2.getAngle(v1,v2)
  return  math.atan2(v2.x - v1.x, v2.y - v1.y) * 180 / math.pi
end

function rotatePoint(pos,deg)
  --return the position after rotation
  return vector2.new(pos.x*math.cos(math.rad(deg)) - pos.y*math.sin(math.rad(deg)), pos.x*math.sin(math.rad(deg)) + pos.y*math.cos(math.rad(deg)))
end

--[[
function vector2:angle(v2)
  --!!!!returns in radians!!!!
  local m = math.sqrt(vector2.magnitudeSq(self) * vector2.magnitudeSq(v2))
  return math.acos(vector2.dot(self,v2) / m)
end
]]
function vector2.perpindicular(v1)
  return vector2.new(v1.y, -v1.x)
end

function vector2.lerp(v1, v2, t) 
   local vx = lerp(v1.x,v2.x,t)
   local vy = lerp(v1.y,v2.y,t)
   
   
   return vector2.new(vx,vy)
end

