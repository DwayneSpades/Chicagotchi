gameSceneManager={}
gameSceneManager.__index=gameSceneManager
gameSceneManager.currentScreen = nil
setmetatable({},gameSceneManager)
screenLoaded=false
--I need to be able to instantiate gamescreens
--update the current game screen 
--when switching game screens delete all data associated with the previous game screen

--make exceptions to catch if the user forgot to give the scene object a intialize(), update(), or draw() methods
--tell the user they goofed up and to add those required functions for scenes to work properly

function gameSceneManager:setScreen(s)

  --unload current screen
  if(self.currentScreen~=nil)then
    self.currentScreen:destroy()
    self.currentScreen = nil
  end
  
  --set and initiate new screen
  self.currentScreen = s
  self.currentScreen:initialize()
  self.currentScreen:update(0)
end

function gameSceneManager:unloadScreen()
  if(self.currentScreen~=nil)then
    self.currentScreen:destroy()
    self.currentScreen = nil
  end
end

function gameSceneManager:update()
  if(self.currentScreen~=nil)then
    self.currentScreen:update()
  end
end

function gameSceneManager:draw()
  if(self.currentScreen~=nil)then
    self.currentScreen:draw()
  end
  
end
