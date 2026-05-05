function drawCircle(position, radius, colorNum)
  local color = colorNum or 1
  --insert myrtle circle draw command
  myrtle.drawCircle(position.x,position.y,radius,"0x"..myPallet[color])
end

function drawRectangle(position,width,height, colorNum)
  local color = colorNum or 1
  --insert myrtle circle draw command
  myrtle.drawRectangle(position.x,position.y,width,height,"0x"..myPallet[color])
end

function drawLine(pos1,pos2,colorNum)
  local color = colorNum or 1
  --insert myrtle circle draw command
  myrtle.drawLine(pos1.x,pos1.y,pos2.x,pos2.y,"0x"..myPallet[color])
end