scene_6={}
scene_6.__index = {}

function scene_6.initialize()
  alarm.new(4000,function() gameSceneManager:setScene(scene_1)  end)
  circleObject = circleCollider.new()
  AABBObject = AABBCollider.new()
  colColor = 1
end

function scene_6:update()
  sinDrive = sinDrive + 0.05
	AABBObject.position.x = sx + 100 + math.sin(sinDrive*2)*20
	AABBObject.position.y = sy + 62 + math.cos(sinDrive*2)*40
  
  circleObject.position.x = sx + 100 + math.sin(sinDrive *-1)*40
	circleObject.position.y = sy + 62 + math.cos(sinDrive *-1)*2
  
  local verdict = circleAABBCollision(circleObject,AABBObject)
  
  if verdict == true then
    colColor = 3
  else
    colColor = 8
  end
  
end

function scene_6:draw()
  drawRectangle(vector2.new(0,64),32,32, 3)
  drawLine(vector2.new(0,64),vector2.new(128,128), 2)
  AABBObject:draw(colColor)
	circleObject:draw(colColor)
end

function scene_6:destroy()
  AABBObject = nil
  circleObject = nil
  colColor=nil
end