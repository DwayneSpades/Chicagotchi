scene_luketest = {}
scene_luketest.__index = {}

function scene_luketest:initialize()
    self.startingPos = vector2.new(32, (myrtle.SCREEN_HEIGHT / 2) - 32)

    self.time = 0
end

function scene_luketest:update()
    self.time = self.time + 1
    self.sprObjPos.y = self.startingPos.y + math.sin(self.time * 0.1) * 16
end

function scene_luketest:draw()
    local pos = vector2.new(self.startingPos.x, self.startingPos.y)

    gatoSprite:draw(pos)

    pos.x = pos.x + 80
    penguinSprite:draw(pos)

    pos.x = pos.x + 80
    bugSprite:draw(pos)

    pos.x = pos.x + 80
    groofSprite:draw(pos)

    pos = nil

    myrtle.setTextColor(myPallet[8])
    myrtle.println("Myrdle Wins")

    myrtle.setTextColor(myPallet[13])
    myrtle.println(tostring(self.sprObjPos.x)..", "..tostring(self.sprObjPos.y))
end

function scene_luketest:destroy()
    self.sprObj = nil
    self.sprObjPos = nil
    self.startingPos = nil
    self.time = nil
end
