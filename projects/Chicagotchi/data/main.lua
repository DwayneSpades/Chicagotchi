--put the C++ wrapper functions into the Myrtle function name table
myrtle={}
myrtle.drawCircle = drawCircle
myrtle.createSprite = createSprite
myrtle.loadPixel = loadPixel
myrtle.drawSprite = drawSprite
myrtle.drawBitmap = drawBitmap

myrtle.getTime = getTime

myrtle.convertHex = convertHex

myrtle.buttonDown = buttonDown
myrtle.buttonUp = buttonUp
myrtle.buttonHeld = buttonHeld
myrtle.buttonUnheld = buttonUnheld

myrtle.require = myrtleRequire
myrtle.print = myrtlePrint
myrtle.println = myrtlePrintln
myrtle.setTextColor = myrtleSetTextColor

myrtle.buttons = {}
myrtle.buttons.D0 = 0
myrtle.buttons.D1 = 1
myrtle.buttons.D2 = 2

myrtle.require("vector2.lua")
myrtle.require("gameSceneManager.lua")
myrtle.require("bitmapReader.lua")
myrtle.require("miscTools.lua")
myrtle.require("alarm.lua")
myrtle.require("alarmManager.lua")
myrtle.require("scene_1.lua")
myrtle.require("scene_2.lua")


engineTime=0;

--example OOP Style Game Object Lau datastructure
testGameObject = {}
testGameObject.__index=testGameObject

function testGameObject.new(spr)
	local instance = setmetatable({}, testGameObject)
	instance.position = vector2.new()
	instance.radius = 16
  instance.sprite = spr or nil
  
	return instance
end

function testGameObject:draw()
	myrtle.drawCircle(self.position.x,self.position.y,self.radius)
end

--1 time execution load function (you could rerun this function to update the code during runtime on the C++)
function myrtle_load()	
	--create test game object to demonstrate easy the to use enviroment for writing game logic 
	circleObject = testGameObject.new()
	
	myPallet =
	{
		"fbf9",
		"9014",
		"df62",
		"16ff",
		"f9a6",
		"fc84",
		"56c0",
		"333c",
		"6004",
		"ea60", --the transparent color
		"042a",
		"098c",
		"ffff",
		"9578",
		"5a32",
		"0865"
	}
	
  --create a sprite object formt the readBMP and return it
  --myrtle.print("\n")
  --myrtle.print("hello: "..myrtle.getTime())
	gatoSprite = readBMP("Gato_Roboto.bmp",myPallet)
  penguinSprite = readBMP("pengin1.bmp",myPallet)
  groofSprite = readBMP("groof.bmp",myPallet)
  bugSprite = readBMP("Bug.bmp",myPallet)
  grombSprite = readBMP("grombles.bmp",myPallet)
  FlowerSprite = readBMP("Flowerguy_Head.bmp",myPallet)
  astroSprite = readBMP("Chicagotchi_Test_AstronautFella_Combined.bmp",myPallet)
  
  --load scene example
  gameSceneManager:setScene(scene_1)
  
  collectgarbage("collect")
end

sinDrive=0


sx = 0;
sy = 0;
engineTime=0
function myrtle_update()
  engineTime = myrtle.getTime()
  gameSceneManager:update()

	runGarbageCollector()
end

function myrtle_draw()
	--draw things here
	circleObject:draw()
	--myrtle.drawCircle(32,32,32)
	--drawSprite(testSprite,spriteObject.position+vector2.new(0,0))
	myrtle.println('Memory actually used (in kB): '..string.format("%i",collectgarbage('count')) )
  
  myrtle.println("engine time: "..engineTime)
	
	gameSceneManager:draw()
  
  
end

--call update from C++ loop