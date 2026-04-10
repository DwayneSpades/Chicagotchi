require("devices")
local bman = require("lib.simplebutton.module")

local border = 16
local buttonCellWidth = 128

local init = true

task = nil

local myrtle = {}

function pushButton(name, onClick)
	local count = #bman.Buttons
    bman.new(name, border + buttonCellWidth * count, border).onClick = onClick
end

function copyMain()
	-- copy source from main
	os.execute("mkdir temp")
	os.execute("xcopy /E /I /Y .\\..\\..\\projects\\Chicagotchi\\data\\ .\\temp\\")

	inject_top("./simstamp.lua", "./temp/main.lua")
 	myrtle = require("temp.main")
end

function inject_top(inject_file, target_file)
    local inject = io.open(inject_file, "r")
    local target = io.open(target_file, "r")

    local lines = {}

    -- write inject first
    for line in inject:lines() do
        lines[#lines+1] = line.."\n"
    end
    inject:close()

    -- write target after
    for line in target:lines() do
        lines[#lines+1] = line.."\n"
    end
    target:close()

    local out = io.open(target_file, "w")
    for _, line in ipairs(lines) do
    	out:write(line)
    end
    out:close()
end

function setupButtons()
	pushButton("Sync", function()
		init = false
		task = function()
			copyMain()
			init = true
		end
	end)

	pushButton("Add Device", function()
		addDevice()
	end)

	pushButton("Remove Device", function()
		addDevice()
	end)
end

function setup()
	copyMain()

	setupButtons()

	init = true
end

function love.load()
    bman.default.width = 100
    bman.default.height = 40
    bman.default.alignment = 'center'

    if (init) then
    	setupButtons()
    end
end

function love.mousepressed(x, y, msbutton, istouch, presses)
    bman.mousepressed(x, y, msbutton)
    devicesHandleMousePressed(x, y)
end

function love.mousereleased(x, y, msbutton, istouch, presses)
    bman.mousereleased(x, y, msbutton)
	devicesHandleMouseReleased()
end

function love.update(dt)
    bman.update(dt)

    if (task ~= nil) then
    	task()
    	task = nil
    end

    updateDevices()
end

function love.draw()
	love.graphics.clear(0.5, 0.5, 0.5, 1)

	if (not init) then
		love.graphics.print("Copying files from main directory...", 400, 300)
		task = setup
	else 
		love.graphics.print("Hello World", 400, 300)
	end

	drawDevices()

    bman.draw()
end

