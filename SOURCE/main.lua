import "CoreLibs/graphics"

gfx = playdate.graphics


hasRendered = nil
centerx = nil
centery = nil
width = nil

optionDrawUI = true
optionDrawFPS = false

function resetZoom()
    hasRendered = false
    centerx = -0.75
    centery = 0
    width = 3.5
end

resetZoom()

function drawFPS()
    if optionDrawFPS then
        playdate.drawFPS(0, 0)
    end
end

function drawUI()
    if not optionDrawUI then
        return
    end

    local font = gfx.getSystemFont()
    local padding = 6
    local textHeight = font:getHeight()

    -- Left bottom hint
    local leftText = "✛ *move*"
    local leftTextWidth, _ = gfx.getTextSize(leftText)
    gfx.setColor(gfx.kColorWhite)
    gfx.fillRoundRect(0 - 42, 240 - 2 * padding - textHeight, leftTextWidth + 2 * padding + 42,
        2 * padding + textHeight + 42, padding)
    gfx.setColor(gfx.kColorBlack)
    gfx.setLineWidth(3)
    gfx.drawRoundRect(0 - 42, 240 - 2 * padding - textHeight - 2, leftTextWidth + 2 * padding + 42 + 2,
        2 * padding + textHeight + 42, padding)
    gfx.setLineWidth(1)
    gfx.drawText(leftText, padding, 240 - padding - textHeight)

    -- Right bottom hint
    local rightText = "🎣 *zoom*  Ⓑ *reset*"
    local rightTextWidth, _ = gfx.getTextSize(rightText)
    gfx.setColor(gfx.kColorWhite)
    gfx.fillRoundRect(400 - 2 * padding - rightTextWidth, 240 - 2 * padding - textHeight,
        rightTextWidth + 2 * padding + 42, 2 * padding + textHeight + 42, padding)
    gfx.setColor(gfx.kColorBlack)
    gfx.setLineWidth(3)
    gfx.drawRoundRect(400 - 2 * padding - rightTextWidth - 2, 240 - 2 * padding - textHeight - 2,
        rightTextWidth + 2 * padding + 42, 2 * padding + textHeight + 42, padding)
    gfx.setLineWidth(1)
    gfx.drawTextAligned(rightText, 400 - padding, 240 - padding - font:getHeight(), kTextAlignment.right)
end

function playdate.update()
    if hasRendered then
        drawFPS()
        return
    end
    hasRendered = true

    local startx = centerx - width / 2
    local stopx = centerx + width / 2
    local starty = centery - (width * 3 / 5) / 2
    local stopy = centery + (width * 3 / 5) / 2

    gfx.clear()
    playdate.resetElapsedTime()
    drawNativeMandelbrot(startx, starty, stopx, stopy)
    print(playdate.getElapsedTime())

    drawFPS()
    drawUI()
end

function panOffset()
    return width / 10
end

function zoomOffset()
    return width / 100
end

function playdate.leftButtonDown()
    hasRendered = false
    centerx -= panOffset()
end

function playdate.rightButtonDown()
    hasRendered = false
    centerx += panOffset()
end

function playdate.upButtonDown()
    hasRendered = false
    centery -= panOffset()
end

function playdate.downButtonDown()
    hasRendered = false
    centery += panOffset()
end

function playdate.BButtonDown()
    resetZoom()
end

function playdate.AButtonDown()
    hasRendered = false
end

function playdate.cranked(change, acceleratedChange)
    hasRendered = false
    for _ = 0, math.abs(change) do
        if change < 0 then
            width += zoomOffset()
        else
            width -= zoomOffset()
        end
    end
end

-- Menu options
menu = playdate.getSystemMenu()

menu:addCheckmarkMenuItem("Show UI", optionDrawUI, function(value)
    hasRendered = false
    optionDrawUI = value
end)


menu:addCheckmarkMenuItem("Draw FPS", optionDrawFPS, function(value)
    hasRendered = false
    optionDrawFPS = value
end)


-- menu:addMenuItem("About", function()
--end)
