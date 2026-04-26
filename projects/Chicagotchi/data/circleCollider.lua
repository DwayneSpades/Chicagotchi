circleCollider={}
circleCollider.__index = circleCollider

function circleCollider.new(pos,r)
  local instance = setmetatable({}, circleCollider)
  
  instance.radius = r
  instance.collided = false
  instance.position = pos
  instance.prevPos = pos
  
  return instance
end

function circleCollider:circleToCircle(c)
  local dist = vector2.magnitude(self.position - c.position)
  local comRad = self.radius + c.radius
  
  if dist <= comRad then
    self.collided = true
    c.collided = true
    --print(collided)
    return true
  end
  
  --later we could run a raycast check from the prev and current positions
  --if the objects go so fast they clip throuhg each other

  return false
end

function circleCollider:raycastCheck()
  
end

function circleCollider:pointInCircle(p)
  d = distance(self.position,p)
  return d <= self.radius
end

function circleCollider:setPosition(pos)
  self.position = pos
end

function circleCollider:draw()
  love.graphics.circle("line",self.position.x*scaleFactor,self.position.y*scaleFactor,self.radius*scaleFactor)
end
