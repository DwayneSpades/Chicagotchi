scene_2={}
scene_2.__index = {}

function scene_2.initialize()
  alarm.new(2000,function() gameSceneManager:setScene(scene_3)  end)
  circleObject = gameObject.new()
 
end

function scene_2:update()
  sinDrive = sinDrive + 0.05
	circleObject.position.x = sx + 100 + math.sin(sinDrive)*20
	circleObject.position.y = sy + 32 + math.cos(sinDrive)*20
end

function scene_2:draw()
  
  circleObject:draw(1)
	bugSprite:draw(circleObject.position + vector2.new(-64,0))
	penguinSprite:draw(circleObject.position + vector2.new(-32,32))
	FlowerSprite:draw(circleObject.position + vector2.new(32,32))
	grombSprite:draw(circleObject.position + vector2.new(-64,32))
end

function scene_2:destroy()
  circleObject = nil
  
end