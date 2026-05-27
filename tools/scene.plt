${SCENE_NAME} = {}
${SCENE_NAME}.__index = {}

function ${SCENE_NAME}:initialize()
    self.sprObj = groofsprObj
    self.sprObjPos = vector2:new()
end

function ${SCENE_NAME}:update()
    self.sprObj
end

function ${SCENE_NAME}:draw()
    self.sprObj:draw()
end

function ${SCENE_NAME}:destroy()
    self.sprObj = nil
    self.sprObjPos = nil
end
