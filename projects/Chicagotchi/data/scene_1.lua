scene_1={}
scene_1.__index = {}



function scene_1.initialize()
 alarm.new(4000,function() gameSceneManager:setScene(scene_2)  end)
 
end

function scene_1:update()
  sinDrive = sinDrive - 0.05
	circleObject.position.x = sx + 100 + math.sin(sinDrive)*20
	circleObject.position.y = sy + 32 + math.cos(sinDrive)*20
end

function scene_1:draw()
  gatoSprite:draw(circleObject.position)
  groofSprite:draw(circleObject.position+vector2.new(32,0))
	astroSprite:draw(circleObject.position + vector2.new(32,32))
end
