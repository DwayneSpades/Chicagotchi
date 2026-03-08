--put the C++ wrapper functions into the Myrtle function name table
myrtle={}
myrtle.drawCircle = drawCircle
myrtle.drawPixel = drawPixel
myrtle.convertHex = convertHex

myrtle.require = myrtleRequire
myrtle.print = myrtlePrint
myrtle.println = myrtlePrintln
myrtle.setTextColor = myrtleSetTextColor

myrtle.require("vector2.lua")

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

function readBMP(fileName, pallet)
	--myrtle.println("reading bmp")
	local f = io.open("/littlefs/"..fileName, "rb")
	
	--myrtle.println("")
    local block = 1
	--bmp header is 54 bytses. 118 byte header for 4bpp format
	--bmp width and height must be a multiple of 4 to prevent padding in the file so it can be read back simpler
	--each pixel in 24bpp bmp is 24 bits long ((B)00,(G)00,(R)00) and must be combined into a single HEX CODE for the color to be displayed by gfx correctly
	local index = 0 
	local index2 = 0 
	local index3 = 0
	local hexCode = ""
	local fileHexCode = ""
	local width = 0
	local height = 0
	local wCounter = 0
	local hCounter = 0
	local fileSize = 0
	local indexPallet = pallet or {}
	local palletIndex = 0
	
	--read file header info
	while true  do
      local bytes = f:read(block)
      if not bytes then break end
	  
	  if(index == 18)then
		for b in string.gfind(bytes, ".") do
			width = myrtle.convertHex(string.format("%02X", string.byte(b)))
			--myrtle.println("width px: "..width)
			
		end
	  end
	  if(index == 22)then
		for b in string.gfind(bytes, ".") do
			height = myrtle.convertHex(string.format("%02X", string.byte(b)))
			--myrtle.println("height px: "..height)
		end
	  end
	  
	  --TODO: add read for pixel pallet indexs here
	  if(index >= 54 and index < 118)then
		for b in string.gfind(bytes, ".") do
			hexCode = hexCode..string.format("%02X", string.byte(b))
		end
		
		index2 = index2 + 1 
		if index2 == 3 then 
			if #indexPallet ~= 16 then
				table.insert(indexPallet, hexCode)
			end
			palletIndex = palletIndex + 1
			myrtle.setTextColor("0x"..indexPallet[palletIndex])
			
		end
		if index2 == 4 then
			
			myrtle.println("0x"..indexPallet[palletIndex])
			hexCode = ""
			index2 = 0
		end
	  end
	  
	  if(index == 118)then
		hexCode = ""
		index2 = 0
	  end
	  
	  if(index > 118 ) then
	  
		  for b in string.gfind(bytes, ".") do
			hexCode = hexCode..string.format("%02X", string.byte(b))
		  end
		  index2 = index2 + 1 
		  if index2 == 1 then 
			--split the hex code ot read bit by bit. lua only can read as small as 1 byte at a time.
			if hexCode:sub(1,1) ~= "9" then
				myrtle.drawPixel(100+wCounter,height-hCounter,"0x"..indexPallet[myrtle.convertHex(hexCode:sub(1,1))+1])
			end
			
			if hexCode:sub(2,2) ~= "9" then
				myrtle.drawPixel(100+(wCounter+1),height-hCounter,"0x"..indexPallet[myrtle.convertHex(hexCode:sub(2,2))+1])
			end
			
			index2 = 0
			index3 = index3 + 2
			wCounter = wCounter + 2 
			--myrtle.setTextColor("0x"..hexCode)
			
			--printing out the bits on screen
			--myrtle.print(hexCode:sub(1,1))
			--myrtle.print(" ")
			--myrtle.print(hexCode:sub(2,2))
			--myrtle.print(" ")
			
			--we've reached a row read.
			--read the next row of pixels.
			--later we can devide this by  a cellsize for reading animations.
			if index3 == width then
				
				--myrtle.print("\n")
				--draw a pixel at the curent whCounter position
				hCounter = hCounter + 1
				wCounter = 0
				index3 = 0
			end
			hexCode = ""
		  end
		   
	  end
	  index = index + 1
    end
	
	io.close(f)
	
	
	--myrtle.print("successfully read bmp")
end

--1 time execution load function (you could rerun this function to update the code during runtime on the C++)
function myrtle_load()	
	--create test game object to demonstrate easy the to use enviroment for writing game logic 
	spriteObject = testGameObject.new()
	local myPallet =
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
		"ea60",
		"042a",
		"098c",
		"ffff",
		"9578",
		"5a32",
		"0865"
	}
	readBMP("Gato_Roboto.bmp")
end

sinDrive=0
function myrtle_update()
	
	sinDrive = sinDrive + 0.05
	spriteObject.position.x = 100 + math.sin(sinDrive)*20
	spriteObject.position.y = 32 + math.cos(sinDrive)*20
end

function myrtle_draw()
	--draw things here
	spriteObject:draw()
	myrtle.drawCircle(32,32,32)
end

--call update from C++ loop