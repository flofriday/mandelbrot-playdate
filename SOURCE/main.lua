gfx = playdate.graphics

hasRendered = false
centerx = -0.75
centery = 0
width = 3.5
function playdate.update()
    if hasRendered then
        playdate.drawFPS(0, 0)
        return
    end
    hasRendered = true

    local startx = centerx - width / 2
    local stopx = centerx + width / 2
    local starty = centery - (width * 3 / 5) / 2
    local stopy = centery + (width * 3 / 5) / 2

    gfx.clear()
    print(getBuildTime())
    playdate.resetElapsedTime()
    drawNativeMandelbrot(startx, starty, stopx, stopy)
    print(playdate.getElapsedTime())
    playdate.drawFPS(0, 0)
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
    -- Resets the zoom / pan
    hasRendered = false
    centerx = -0.75
    centery = 0
    width = 3.5
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
