function lerp(ob1, ob2, t) 
  return ((1-t) * ob1 + t *  ob2)
end

function angleLoop(angle)
  if angle>180 then
    angle = 0
  end
  if angle < 0.00001  then
    angle = 180
  end
  
end

function lerpAngle(angle1,angle2,t)
  return ((1-t) * angle1 + t *  angle2)
end

function interpolate(obj1, obj2, t)
  return (obj1 + (obj2 - obj1) * t)
end

function roundNumber(n)
  return math.floor(n+0.4999999999999994)
end


function roundVector2(n)
  return vector2.new(roundNumber(n.x),roundNumber(n.y))
end

--[[
function AABBCollision(pos1,w1,h1,pos2,w2,h2)

  return pos1.x < pos2.x+w2 and
         pos2.x < pos1.x+w1 and
         pos1.y < pos2.y+h2 and
         pos2.y < pos1.y+h1
end
]]

function AABBCollision(rect1,rect2)
  local aMin = rect1:getMin()
  local aMax = rect1:getMax()
  
  local bMin = rect2:getMin()
  local bMax = rect2:getMax()
  
  overX =  ((bMin.x <= aMax.x) and (aMin.x <= bMax.x))
  overY =  ((bMin.y <= aMax.y) and (aMin.y <= bMax.y))
  
  return overX==true and overY==true
  
end

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

function triangleArea(p1,p2,p3)
  return (p1.x - p3.x) * (p2.y - p3.y) - (p1.y - p3.y) * (p2.x - p3.x)
end

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

--might need ot break this function up to give me intersection point and bool detection
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
      return p
    end
  end
  --print("no intersect")
  return nil
end

contactPair = {index=0,contactTime=0}

contactPair._index = contactPair 
function contactPair.new(i,cTime)
  local instance = setmetatable({}, contactPair)
  
  instance.index = i
  instance.contactTime = cTime or 1
  
  return instance
end

 
function rayRectCollision(rayOrigin,rayDir, rect)
  --comput near and far collision points
  local tNear = (rect.position - rayOrigin) / rayDir
  local tFar = (rect.position +vector2.new(rect.width,rect.height) - rayOrigin) / rayDir
  
  --checks for nan which is invalid numbers
  if(tNear.x~=tNear.x or tNear.y~=tNear.y)then
    return false,nil
  end
  if(tFar.x~=tFar.x or tFar.y~=tFar.y)then
    return false,nil
  end
  
  if(tNear.x > tFar.x)then
    local nTemp = tNear.x
    local fTemp = tFar.x
    
    tNear.x = fTemp
    tFar.x = nTemp
    --print("swapping")
    --print(tNear.x)
    --print(tFar.x)
    
  end
  if(tNear.y > tFar.y)then
    local nTemp = tNear.y
    local fTemp = tFar.y
    
    tNear.y = fTemp
    tFar.y = nTemp
    --print("swapping")
    --print(tNear.y)
    --print(tFar.y)
    
  end
  
  if(tNear.x > tFar.y or tNear.y > tFar.x)then
    return false
  end
  
  local tHitNear = math.max(tNear.x, tNear.y)
  local tHitFar = math.max(tFar.x, tFar.y)
  
  if(tHitFar < 0)then
    return false,nil
  end
  
  local contactPoint = rayOrigin + (rayDir * tHitNear)
  local contactNormal = vector2.new(0,0)
  
  --**NOTE**: if tHitNear is less than 1 in a collision it means the ray is actually touching the rectangle
  if(tHitNear < 1)then
    love.graphics.setColor(1,1,1)
    love.graphics.circle("fill",contactPoint.x,contactPoint.y,4)
  end
  

  if(tNear.x > tNear.y)then
    if(rayDir.x < 0)then
      contactNormal = vector2.new(1,0)
    else
      contactNormal = vector2.new(-1,0)
    end
  elseif(tNear.x < tNear.y) then
    if(rayDir.y < 0)then
      contactNormal = vector2.new(0,1)
    else
      contactNormal = vector2.new(0,-1)
    end
    
  end
  
  colInfo = rayCollisionInfo.new(contactPoint,contactNormal, tHitNear)
  
  if(tHitNear <= 1)then
    --print("contactTime: "..colInfo.contactTime)
    return true, colInfo
  end
  
  return false, nil
end

rectCollisionInfo={}
rectCollisionInfo.__index = rectCollisionInfo

function rectCollisionInfo.new()
  local instance = setmetatable({}, rectCollisionInfo)
  
  instance.contactPoint = nil
  instance.contactNormal = nil
  instance.contactTime = 0
  
end

function rectCollision(rect1, rect2,vel,deltaTime)
  if(vel.x == 0 and vel.y == 0) then
    return false
  end
  
  local ePos = rect2.position - (vector2.new(rect1.width,rect1.height)/2)
  local expandedRect = AABB_Collider.new(ePos.x,ePos.y,rect1.width + rect2.width, rect1.height + rect2.height)
  
  local result,colInfo = rayRectCollision(rect1.position + vector2.new(rect1.width,rect1.height)/2, vel * deltaTime, expandedRect)
  if(result) then
    --print("contactTime: "..colInfo.contactTime)
    return result,colInfo
  end
  
  return false,nil
end

function circleCollision(ob1,ob2)
  local dist = vector2.magnitude(ob1.position - ob2.position)
  local combinedRadius = ob1.radius + ob2.radius
  
  if dist <= combinedRadius then
    return true
  end
  
  --later we could run a raycast check from the prev and current positions
  --if the objects go so fast they clip throuhg each other

  return false
end

function circleCollisionInfo(ob1,ob2)
  local dir = vector2.normalized(ob2.position-ob1.position)
  local perpindicular = vector2.perpindicular(dir)
  
  return dir,perpindicular
end

function circleRectCollision(circle,rect)
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
  
  local dist = vector2.magnitude(circle.position - posEdge)
  
  if(dist <= circle.radius) then
    --it shuold give info on what side we collided with...
    return true
  end
  
  return false
  
end

function circleRectCollisionInfo(circle,rect)
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

function pointOverRect(mPos,v)
 return mPos.x > v.position.x     and 
        mPos.x < v.position.x+v.width and
        mPos.y > v.position.y     and
        mPos.y < v.position.y+v.height
end

function distance(ob1,ob2)
  return math.sqrt(((ob1.x-ob2.x)^2)+((ob1.y-ob2.y)^2))
end

function math.clamp(x, min, max)
  if(x <= min) then
    return min
  elseif x >= max then
    return max
  end
  
  return x
end