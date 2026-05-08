scene_luketest = {}
scene_luketest.__index = {}

function scene_luketest:initialize()
    self.sprObj = groofSprite
    self.startingPos = vector2.new((myrtle.SCREEN_WIDTH / 2) - 32, (myrtle.SCREEN_HEIGHT / 2) - 32)
    self.sprObjPos = vector2.new(self.startingPos.x, self.startingPos.y)

    self.time = 0
end

function scene_luketest:update()
    self.time = self.time + 1
    self.sprObjPos.y = self.startingPos.y + math.sin(self.time * 0.1) * 16
end

function scene_luketest:draw()
    self.sprObj:draw(self.sprObjPos)

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
