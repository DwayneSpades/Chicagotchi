scene_testGame={}
scene_testGame.__index = {}

screenShakeOn = false
shakeAlarm = nil

function screenShake(time)
  
  if shakeAlarm==nil then
    shakeAlarm = alarm.new(time,function() screenShakeOn = false; myrtle.setScreenPos(0,0) end)
  else
    --reset timer
    shakeAlarm:destroy()
    shakeAlarm = alarm.new(time,function() screenShakeOn = false; myrtle.setScreenPos(0,0) end)
  end
  screenShakeOn = true
end

function shakeScreen()
  local x = math.random(-16,16)
  local y = math.random(-16,16)
  myrtle.setScreenPos(x,y)
end

function scene_testGame.initialize()
  --screen shakes until the  alarm triggers
  
  boundary1 = AABBCollider.new(vector2.new(0,0),240,16)
  boundary3 = AABBCollider.new(vector2.new(0,0),16,135)
  boundary2 = AABBCollider.new(vector2.new(240-16,0),16,135)
  boundary4 = AABBCollider.new(vector2.new(0,135-16),240,16)
  
  blocks = 
  {
    boundary1,
    boundary2,
    boundary3,
    boundary4
  }
  
  --always mix up the randomness
  math.randomseed(engineTime)
  enemies={enemy.new(vector2.new(math.random(100,100),math.random(100,100)))}
  
  playerPet = myPet.new()
  
  --list to process player collision with level
  players={}
  
  table.insert(players,playerPet)
  for i,v in ipairs(enemies)do
    table.insert(players,v)
  end
  
  endGame = false
  
  --coin power ups
  
end

function scene_testGame:update()
  if screenShakeOn==true then
    shakeScreen()
  end
  
  --update players and collision
  for i,p in ipairs(players)do
    if p.health > 0 then
      p:update()
      self:runCollision(p)
    end
  end
  
  --run player to player collisions
  if playerPet ~=nil then
    for i,v in pairs(enemies)do
      --collision check against enemies
      if circleCollision(playerPet.collider,v.collider)then
        local dir,perp =  circleCollisionInfo(playerPet.collider,v.collider)
        playerPet.position = v.position-dir*(v.collider.radius+playerPet.collider.radius)
        
        local reflection = vector2.normalized(vector2.normalized(playerPet.dir - (dir*vector2.dot(playerPet.dir,dir))*2 ))
        local reflection2 = vector2.normalized(vector2.normalized(v.dir - (dir*vector2.dot(v.dir,dir))*2 ))
        
        playerPet.dir = reflection
        v.dir = reflection2
        
        v.speed = 5
        playerPet.speed = 5
        
        v.health = v.health - math.random(1,5)
        playerPet.health = playerPet.health -math.random(1,5)
        
        screenShake(100)
        
        if playerPet~=nil and playerPet.health <=0 then
          playerPet = nil
        end
        
        if(v.health==0 )then
          enemies={}
        end
        
      end
    end
  end
  
end

function scene_testGame:draw()
  
  for i, v in pairs(blocks) do 
    v:draw(8)
  end
  for i, v in pairs(players) do 
    if v.health > 0 then
      v:draw()
    end
  end
  
  if(#enemies<=0) then
    myrtle.setTextColor(myPallet[8])
    myrtle.println("Myrdle Wins")
    if endGame == false then
      endGame = true
      alarm.new(2000,function() gameSceneManager:setScene(scene_testGame)  end)
    end
  end
  
  if playerPet == nil then
    myrtle.setTextColor(myPallet[5])
    myrtle.println("Groof Wins")
    if endGame == false then
      endGame = true
      alarm.new(2000,function() gameSceneManager:setScene(scene_testGame)  end)
    end
  end
  
end

function scene_testGame:destroy()
  playerPet = nil
  players = nil
  enemies = nil
  
  blocks = nil
  boundary1=nil
  boundary2=nil
  boundary3=nil
  boundary4=nil
  
  endGame = nil
end

function scene_testGame:runCollision(player)
  for i, v in pairs(blocks) do
    if circleAABBCollision(player.collider,v) then
      local pos,dir  = circleAABBCollisionInfo(player.collider,v)
      player.position = pos+dir*(player.collider.radius)

      if(dir.x<0)then
        player.dir.x=player.dir.x*-1
      elseif (dir.x>0) then
        player.dir.x=player.dir.x*-1
      end
      
      if(dir.y<0)then
        player.dir.y=player.dir.y*-1
        
      elseif (dir.y>0) then
        player.dir.y=player.dir.y*-1
      end
    end
  end
  
end
