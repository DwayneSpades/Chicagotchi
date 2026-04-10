--put the C++ wrapper functions into the Myrtle function name table
myrtle={}
myrtle.drawCircle = drawCircle
myrtle.createSprite = createSprite
myrtle.loadPixel = loadPixel
myrtle.drawSprite = drawSprite
myrtle.drawBitmap = drawBitmap

myrtle.convertHex = convertHex

myrtle.require = myrtleRequire
myrtle.print = myrtlePrint
myrtle.println = myrtlePrintln
myrtle.setTextColor = myrtleSetTextColor

myrtle.require("vector2.lua")
myrtle.require("gameSceneManager.lua")
myrtle.require("bitmapReader.lua")
myrtle.require("miscTools.lua")
myrtle.require("packets.lua")

engineTime=0;

--example OOP Style Game Object Lau datastructure
testGameObject = {}
testGameObject.__index=testGameObject

function testGameObject.new()
	local instance = setmetatable({}, testGameObject)
	instance.position = vector2.new()
	instance.radius = 16
	return instance
end

function testGameObject:draw()
	myrtle.drawCircle(self.position.x,self.position.y,self.radius)
end

--1 time execution load function (you could rerun this function to update the code during runtime on the C++)
function myrtle_load()	
	--create test game object to demonstrate easy the to use enviroment for writing game logic 
	spriteObject = testGameObject.new()
	
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
	
	testSprite = readBMP("Gato_Roboto.bmp",myPallet)
	testSprite=nil
end

sinDrive=0

function myrtle_on_streetpass(peer)
	-- get peer info
	-- print(peer.name) -- ? maybe ?
end

function myrtle_update()
	
	sinDrive = sinDrive + 0.05
	spriteObject.position.x = 100 + math.sin(sinDrive)*20
	spriteObject.position.y = 32 + math.cos(sinDrive)*20
	
	myrtle.testPacket({
		obj1 = 5,
		obj2 = 3.5,
		obj3 = "This is a string",
		obj4 = {
			obj1 = nil,
			obj2 = "sneerf"
		}
	})

	runGarbageCollector()
end

function myrtle_draw()
	--draw things here
	spriteObject:draw()
	--myrtle.drawCircle(32,32,32)
	--drawSprite(testSprite,spriteObject.position+vector2.new(0,0))
	myrtle.println('Memory actually used (in kB): '..string.format("%i",collectgarbage('count')) )
	
	--drawBitmap("testBMP",vector2.new(50,50))
	
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-90,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-60,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,30))
	
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-60,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-60,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,-30))
	
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-90,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,0))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-60,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,-30))
	
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-60,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-60,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,-30))
	
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-60,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(60,-10))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-60,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(-30,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(0,-30))
	drawDrawable("Gato_Roboto.bmp",spriteObject.position+vector2.new(30,-30))
	
	
end

--call update from C++ loop