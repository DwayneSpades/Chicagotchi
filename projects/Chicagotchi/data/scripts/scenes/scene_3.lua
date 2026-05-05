scene_3={}
scene_3.__index = {}

function scene_3.initialize()
  alarm.new(4000,function() gameSceneManager:setScene(scene_4)  end)
  circleObject2 = circleCollider.new()
  circleObject = circleCollider.new()
  colColor = 1
end

function scene_3:update()
  sinDrive = sinDrive + 0.05
	circleObject.position.x = sx + 100 + math.sin(sinDrive)*20
	circleObject.position.y = sy + 62 + math.cos(sinDrive)*20
  
  circleObject2.position.x = sx + 100 + math.sin(sinDrive *-1)*20
	circleObject2.position.y = sy + 62 + math.cos(sinDrive *-1)*20
  
  local verdict = circleCollision(circleObject,circleObject2)
  
  if verdict == true then
    colColor = 3
  else
    colColor = 8
  end
  
end

function scene_3:draw()
  drawRectangle(vector2.new(0,64),32,32, 5)
  drawLine(vector2.new(0,64),vector2.new(128,128), 3)
  circleObject:draw(colColor)
	circleObject2:draw(colColor)
end

function scene_3:destroy()
  circleObject = nil
  circleObject2 = nil
  colColor=nil
end