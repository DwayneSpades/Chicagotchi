scene_5={}
scene_5.__index = {}

function scene_5.initialize()
  alarm.new(4000,function() gameSceneManager:setScene(scene_6)  end)
  lineObject2 = lineCollider.new(vector2.new(32,32),vector2.new(64,64))
  lineObject = lineCollider.new(vector2.new(120,80),vector2.new(120,64))
  colColor = 1
end

function scene_5:update()
  sinDrive = sinDrive + 0.05
	lineObject.point2.x = sx + 100 + math.sin(sinDrive*2)*20
	lineObject.point2.y = sy + 62 + math.cos(sinDrive*2)*40
  
  lineObject2.point2.x = sx + 160 + math.sin(sinDrive *-1)*20
	lineObject2.point2.y = sy + 62 + math.cos(sinDrive *-1)*2
  
  local verdict = lineIntersect(lineObject,lineObject2)
  
  if verdict == true then
    colColor = 3
  else
    colColor = 8
  end
  
end

function scene_5:draw()
  drawRectangle(vector2.new(0,64),32,32, 10)
  drawLine(vector2.new(0,64),vector2.new(128,128), 11)
  lineObject:draw(colColor)
	lineObject2:draw(colColor)
end

function scene_5:destroy()
  lineObject = nil
  lineObject2 = nil
  colColor=nil
end