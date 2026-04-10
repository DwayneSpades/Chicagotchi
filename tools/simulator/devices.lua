devices = {}

local draggedItem = 0

local SCREEN_WIDTH = 240
local SCREEN_HEIGHT = 135

local clickOffsetX = 0
local clickOffsetY = 0

local dragStartX = 0
local dragStartY = 0

function addDevice()
	device = {}
	devices[#devices+1] = device

	device.id = #devices
	device.x = 48 * #devices
	device.y = 48 * #devices

	return device
end

function insideDevice(d, x, y)
	return (x > d.x and x < d.x + SCREEN_WIDTH and y > d.y and y < d.y + SCREEN_HEIGHT)
end

function devicesHandleMousePressed(x, y)
	for i, device in ipairs(devices) do
		if (insideDevice(device, x, y)) then
			clickOffsetX = device.x - x
			clickOffsetY = device.y - y

			dragStartX = x
			dragStartY = y

			draggedItem = i
		end
	end
end

function devicesHandleMouseReleased()
	draggedItem = 0
end

function updateDevices()
	local x, y = love.mouse.getPosition()

	if (draggedItem > 0 and draggedItem <= #devices) then
		devices[draggedItem].x = x + clickOffsetX
		devices[draggedItem].y = y + clickOffsetY
	end
end

function drawDevices()
	for i, device in ipairs(devices) do
    	love.graphics.setColor(0, 0, 0)
		love.graphics.rectangle("fill", device.x, device.y, SCREEN_WIDTH, SCREEN_HEIGHT)
		love.graphics.print(device.id, device.x + SCREEN_WIDTH + 16, device.y - 16)
	end
end