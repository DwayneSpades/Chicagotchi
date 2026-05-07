scene_credits={}
scene_credits.__index = {}


function scene_credits.initialize()

end

function scene_credits:update()

end

function scene_credits:draw()
  myrtle.println("\n\nCREDITS:")
  
  for i,v in ipairs(credits) do
    myrtle.println(v)
  end
  
  myrtle.println("\n\n")
  
end

function scene_credits:destroy()
  
end
