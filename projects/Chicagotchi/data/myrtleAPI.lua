--put the C++ wrapper functions into the Myrtle function name table
myrtle={}
myrtle.drawCircle = _drawCircle
myrtle.drawRectangle = _drawRectangle
myrtle.drawLine = _drawLine
myrtle.drawText = _drawText
myrtle.setScreenPos = _setScreenPos

myrtle.createSprite = createSprite
myrtle.loadPixel = loadPixel
myrtle.drawSprite = drawSprite
myrtle.drawBitmap = drawBitmap

myrtle.getTime = getTime

myrtle.convertHex = convertHex

myrtle.buttonDown = buttonDown
myrtle.buttonUp = buttonUp
myrtle.buttonHeld = buttonHeld
myrtle.buttonUnheld = buttonUnheld

myrtle.require = myrtleRequire
myrtle.print = myrtlePrint
myrtle.println = myrtlePrintln

--override print funcitons
print = myrtle.print
println = myrtle.println

myrtle.setTextColor = myrtleSetTextColor

myrtle.sendMessage = sendMessage
myrtle.getPeerCount = getPeerCount
myrtle.getPeerAddr = getPeerAddr

myrtle.buttons = {}
myrtle.buttons.D0 = 0
myrtle.buttons.D1 = 1
myrtle.buttons.D2 = 2