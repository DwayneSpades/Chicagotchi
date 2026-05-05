scene_4={}
scene_4.__index = {}

function scene_4.initialize()
  alarm.new(4000,function() gameSceneManager:setScene(scene_5)  end)
  AABBObject2 = AABBCollider.new()
  AABBObject = AABBCollider.new()
  colColor = 1
end

function scene_4:update()
  sinDrive = sinDrive + 0.05
	AABBObject.position.x = sx + 100 + math.sin(sinDrive*2)*20
	AABBObject.position.y = sy + 62 + math.cos(sinDrive*2)*40
  
  AABBObject2.position.x = sx + 100 + math.sin(sinDrive *-1)*40
	AABBObject2.position.y = sy + 62 + math.cos(sinDrive *-1)*2
  
  local verdict = AABBCollision(AABBObject,AABBObject2)
  
  if verdict == true then
    colColor = 3
  else
    colColor = 8
  end
  
end

function scene_4:draw()
  drawRectangle(vector2.new(0,64),32,32, 8)
  drawLine(vector2.new(0,64),vector2.new(128,128), 3)
  AABBObject:draw(colColor)
	AABBObject2:draw(colColor)
end

function scene_4:destroy()
  AABBObject = nil
  AABBObject2 = nil
  colColor=nil
end