gameSceneManager={}
gameSceneManager.__index=gameSceneManager
gameSceneManager.currentScene = nil
setmetatable({},gameSceneManager)
screenLoaded=false
--I need to be able to instantiate gamescreens
--update the current game screen 
--when switching game screens delete all data associated with the previous game screen

--make exceptions to catch if the user forgot to give the scene object a intialize(), update(), or draw() methods
--tell the user they goofed up and to add those required functions for scenes to work properly

function gameSceneManager:setScene(s)
  --assert(self.currentScene.destroy)
  
  --unload current screen
  if(self.currentScene~=nil)then
    if self.currentScene.destroy~=nil then
      self.currentScene:destroy()
    end
    self.currentScene = nil
  end
  
  --set and initiate new screen
  self.currentScene = s
  --assert(self.currentScene.initialize)
  self.currentScene:initialize()
  self.currentScene:update(0)
end

function gameSceneManager:unloadScreen()
  
  if(self.currentScene~=nil)then
    if self.currentScene.destroy~=nil then
      self.currentScene:destroy()
    end
    self.currentScene = nil
  end
end

function gameSceneManager:update()
  --assert(self.currentScene.update)
  alarmManager:update()
  
  if(self.currentScene~=nil)then
    self.currentScene:update()
  end
  
end

function gameSceneManager:draw()
  --assert(self.currentScene.draw)
  if(self.currentScene~=nil)then
    self.currentScene:draw()
  end
  
end
