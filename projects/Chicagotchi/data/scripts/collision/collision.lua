
--returns lerp position between 2 values or vectors
function lerp(ob1, ob2, t) 
  return ((1-t) * ob1 + t *  ob2)
end
--returns lerps between 2 given angle values
function lerpAngle(angle1,angle2,t)
  return ((1-t) * angle1 + t *  angle2)
end
--interpolate between to given values or vectors
function interpolate(obj1, obj2, t)
  return (obj1 + (obj2 - obj1) * t)
end

--returns number value that's rounded ot the nearest whole number
function roundNumber(n)
  return math.floor(n+0.4999999999999994)
end

--returns a vector2 with values that are rounded ot the nearest whole number
function roundVector2(n)
  return vector2.new(roundNumber(n.x),roundNumber(n.y))
end

--loops angle from 180 to 0 and vica-verca
function angleLoop(angle)
  if angle>180 then
    angle = 0
  end
  if angle < 0.00001  then
    angle = 180
  end
  
end

--returns the distance between 2 positions
function distance(ob1,ob2)
  return math.sqrt(((ob1.x-ob2.x)^2)+((ob1.y-ob2.y)^2))
end

--returns number value clamped between a given min and max
function math.clamp(x, min, max)
  if(x <= min) then
    return min
  elseif x >= max then
    return max
  end
  
  return x
end

--returns boolean stating if a point position is inside a AABB Collider/ rectangle
function pointOverRect(mPos,v)
 return mPos.x > v.position.x     and 
        mPos.x < v.position.x+v.width and
        mPos.y > v.position.y     and
        mPos.y < v.position.y+v.height
end

--returns boolean stating if 2 AABB colliders/rectangles collided
function AABBCollision(rect1,rect2)
  local aMin = rect1:getMin()
  local aMax = rect1:getMax()
  
  local bMin = rect2:getMin()
  local bMax = rect2:getMax()
  
  overX =  ((bMin.x <= aMax.x) and (aMin.x <= bMax.x))
  overY =  ((bMin.y <= aMax.y) and (aMin.y <= bMax.y))
  
  return overX==true and overY==true
  
end

--returns the penetration amount, collision direction, and
function AABBCollisionInfo(rect1,rect2)
  local aMin = rect1:getMin()
  local aMax = rect1:getMax()
  local centerRect1 = aMax/2
  
  --center closer to the left right up or down side of the otehr rectangle?
  --[[
  local lDist = centerRect1.x-rect.position.x
  local rDist = (rect.position.x+rect.width) - centerRect1.x
  local uDist = centerRect1.y-rect.position.y
  local dDist = (rect.position.y+rect.height) - centerRect1.y
  ]]
  local bMin = rect2:getMin()
  local bMax = rect2:getMax()
  
  overX =  ((bMin.x <= aMax.x) and (aMin.x <= bMax.x))
  overY =  ((bMin.y <= aMax.y) and (aMin.y <= bMax.y))
  
  local distA = vector2.magnitude(aMin - bMax)
  local distB = vector2.magnitude(bMin - aMax)
  local penX = 0
  local penY = 0
  
  local pushOut = vector2.new(0,0)
  local sidePen = vector2.new(0,0)
  
  --determine which one is the cause of  the overlap by checking the shortest overlap
  if(distA < distB)then
    --which axis is penetrated the most
    penX = aMin.x - bMax.x
    penY = aMin.y - bMax.y

    sidePen = bMax
    
  else
    penX = bMin.x - aMax.x
    penY = bMin.y - aMax.y
    
    sidePen = bMin
  end
  local outDir = centerRect1 - sidePen
  return vector2.new(penX,penY),outDir,sidePen
  
end

--a helper function to the line to line collision test
function triangleArea(p1,p2,p3)
  return (p1.x - p3.x) * (p2.y - p3.y) - (p1.y - p3.y) * (p2.x - p3.x)
end


--returns the closest point on a line to a givent point position
function closestPointOnLine(point,line)
  local dir = line.point2 - line.point1
  local t = vector2.dot(point-line.point1,dir) / vector2.dot(dir,dir)
  if(t < 0 ) then
    t=0
  end
  if(t>1)then
    t=1
  end
  local closestPoint = line.point1 + (dir*t)
  
  return closestPoint
end

--returns boolean stating if 2 line objects intersect
function lineIntersect(l1,l2)
  --print(l1.point1.y)
  local a1 = triangleArea(l1.point1, l1.point2,l2.point2)
  local a2 = triangleArea(l1.point1, l1.point2,l2.point1)
  
  if(a1 * a2 <= 0) then
    local a3 = triangleArea(l2.point1, l2.point2, l1.point1)
    
    local a4 = a3 + a2 - a1
    
    if(a3 * a4 <= 0) then
      --find the intersection point
      local t = a3 / (a3-a4)
      local p = l1.point1 + (l1.point2 - l1.point1) * t 
      --print("rays intersect")
      --return the calcualted intersection point
      return true
    end
  end
  --print("no intersect")
  return false
end

--returns a boolean stating fi 2 circles are colliding
function circleCollision(ob1,ob2)
  local dist = vector2.magnitude(ob1.position - ob2.position)
  local combinedRadius = ob1.radius + ob2.radius
  
  if dist <= combinedRadius then
    return true
  end

  return false
end

--returns the direciton and the perpindicular direction of the collision
function circleCollisionInfo(ob1,ob2)
  local dir = vector2.normalized(ob2.position-ob1.position)
  local perpindicular = vector2.perpindicular(dir)
  
  return dir,perpindicular
end

--returns a boolean stating if the circle and AABB Collider/rectangle given are colliding
function circleAABBCollision(circle,rect)
  --find the edges to test
  local collisionPoint = vector2.new(circle.position.x,circle.position.y)
  
  if(circle.position.x < rect.position.x) then
    collisionPoint.x = rect.position.x
  elseif (circle.position.x > (rect.position.x + rect.width)) then
    collisionPoint.x = rect.position.x + rect.width
  end
  
  if(circle.position.y < rect.position.y) then
    collisionPoint.y = rect.position.y
  elseif (circle.position.y > (rect.position.y + rect.height)) then
    collisionPoint.y = rect.position.y + rect.height
  end
  
  local dist = vector2.magnitude(circle.position - collisionPoint)
  
  if(dist <= circle.radius) then
    --it shuold give info on what side we collided with...
    return true
  end
  
  return false
  
end

---returns the position and direction of the collision
function circleAABBCollisionInfo(circle,rect)
  --find the edges to test
  local posEdge = vector2.new(circle.position.x,circle.position.y)
  
  if(circle.position.x < rect.position.x) then
    posEdge.x = rect.position.x
  elseif (circle.position.x > (rect.position.x + rect.width)) then
    posEdge.x = rect.position.x + rect.width
  end
  
  if(circle.position.y < rect.position.y) then
    posEdge.y = rect.position.y
  elseif (circle.position.y > (rect.position.y + rect.height)) then
    posEdge.y = rect.position.y + rect.height
  end
  
  local lDist= circle.position.x-rect.position.x
  local rDist= (rect.position.x+rect.width) - circle.position.x
  local uDist= circle.position.y-rect.position.y
  local dDist= (rect.position.y+rect.height) - circle.position.y
  
  local shortest = lDist
  posEdge = vector2.new(rect.position.x,circle.position.y)
  local dir = vector2.new(-1,0)
  
  if shortest > rDist then
    shortest = rDist
    posEdge = vector2.new(rect.position.x+rect.width,circle.position.y)
    dir = vector2.new(1,0)
  end
  
  if shortest > uDist then
    shortest = uDist
    posEdge = vector2.new(circle.position.x,rect.position.y)
    dir = vector2.new(0,-1)
  end
  
  if shortest > dDist then
    shortest = dDist
    posEdge = vector2.new(circle.position.x,rect.position.y+rect.height)
    dir = vector2.new(0,1)
  end
  
  
  --local dir = vector2.normalized(circle.position - posEdge)
  --return collision point and dir

  return posEdge,dir
  
end
