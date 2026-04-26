
pixelObject = {}
pixelObject.__index=pixelObject

function pixelObject.new(pos,color)
	local instance = setmetatable({}, pixelObject)
	instance.position = pos or vector2.new()
	instance.color = color or nil
	
	return instance
end

spriteObject = {}
spriteObject.__index = spriteObject

function spriteObject.new(name)
  local instance = setmetatable({}, spriteObject)
  instance.name = name or "unknown"
  
  return instance
end

function spriteObject:draw(pos)
  drawDrawable(self.name,pos)
end

--load the bmp pixels to memory to be drawn back faster
function readBMP(fileName, pallet)
	local bitmap={}
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
	  
	  if(index >= 118 ) then
	  
		  for b in string.gfind(bytes, ".") do
			hexCode = hexCode..string.format("%02X", string.byte(b))
		  end
		  index2 = index2 + 1 
		  if index2 == 1 then 
			--split the hex code ot read bit by bit. lua only can read as small as 1 byte at a time.
			--store the bits hex codes into a single array
			if hexCode:sub(1,1) then
				table.insert(bitmap,pixelObject.new(vector2.new(wCounter,height-hCounter),"0x"..indexPallet[myrtle.convertHex(hexCode:sub(1,1))+1]))
				--myrtle.drawPixel(wCounter,height-hCounter,"0x"..indexPallet[myrtle.convertHex(hexCode:sub(1,1))+1])
			end
			
			if hexCode:sub(2,2) then
				table.insert(bitmap,pixelObject.new(vector2.new((wCounter+1),height-hCounter),"0x"..indexPallet[myrtle.convertHex(hexCode:sub(2,2))+1]))
				--myrtle.drawPixel((wCounter+1),height-hCounter,"0x"..indexPallet[myrtle.convertHex(hexCode:sub(2,2))+1])
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
	
	--create sprite object to hold the pixel data on the CPP side in the sprite dictionary
	myrtle.createSprite(fileName,#bitmap,width,height)
	index=nil
	--load bitmap pixel data to cpp side here
	for i=1,#bitmap do
		local index = (#bitmap+1) - i
		myrtle.loadPixel(fileName, i-1, bitmap[index].color)
	end
	
  bitmap = nil
  --collect garbage in between loads to not overwhelm the Runtime memory limit with the size of the garbage collector
  collectgarbage("collect")
  
	myrtle.print("read bmp: "..fileName.." - "..string.format("%i",collectgarbage('count')).." kb\n")

	return spriteObject.new(fileName)
	
end

function drawDrawable(filename,pos)
	--I want to pass the table to CPP to draw the bit map straight up
	myrtle.drawSprite(filename,pos.x,pos.y)
end
