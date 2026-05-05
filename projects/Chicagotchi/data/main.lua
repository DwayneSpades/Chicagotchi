

engineTime=0;

sinDrive=0
sx = 0;
sy = 0;

--example OOP Style Game Object Lau datastructure
gameObject = {}
gameObject.__index=gameObject

function gameObject.new(spr)
	local instance = setmetatable({}, gameObject)
	instance.position = vector2.new()
	instance.radius = 16
  instance.sprite = spr or nil
  
	return instance
end

function gameObject:draw(color)
	myrtle.drawCircle(self.position.x,self.position.y,self.radius,"0x"..myPallet[color])
end

--1 time execution load function (you could rerun this function to update the code during runtime on the C++)
function myrtle_load()	
	--create test game object to demonstrate easy the to use enviroment for writing game logic 
	
	
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
  --for dir in io.popen("/littlefs/"):lines() do myrtle.print("log- "..dir.."/n") end
  
	gatoSprite = readBMP("Gato_Roboto.bmp",myPallet)
  penguinSprite = readBMP("pengin1.bmp",myPallet)
  groofSprite = readBMP("groof.bmp",myPallet)
  bugSprite = readBMP("Bug.bmp",myPallet)
  grombSprite = readBMP("grombles.bmp",myPallet)
  FlowerSprite = readBMP("Flowerguy_Head.bmp",myPallet)
  --astroSprite = readBMP("Chicagotchi_Test_AstronautFella_Combined.bmp",myPallet)
  
  --load scene example
  gameSceneManager:setScene(scene_1)
  
  collectgarbage("collect")
end

function myrtle_update()
  engineTime = myrtle.getTime()
  gameSceneManager:update()
	runGarbageCollector()
end

function myrtle_draw()
	--draw things here
	
	--myrtle.drawCircle(32,32,32)
  
	myrtle.println('Memory actually used (in kB): '..string.format("%i",collectgarbage('count')) )
  --myrtle.println("engine time: "..engineTime)

	gameSceneManager:draw()
  
end

--call update from C++ loop