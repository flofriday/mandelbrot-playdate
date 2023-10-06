gfx = playdate.graphics
MAX_ITERATIONS = 64

function createComplex(re, im)
    local complex = {
        re = re,
        im = im,

        square = function(self)
            local reSquared = self.re * self.re
            local imSquared = self.im * self.im
            return createComplex(reSquared - imSquared, 2.0 * self.re * self.im)
        end,

        plus = function(self, other)
            return createComplex(self.re + other.re, self.im + other.im)
        end
    }
    return complex
end

function drawMandelbrot()
    gfx.setColor(gfx.kColorBlack)

    local start = createComplex(-2.5, 1.0)
    local stop = createComplex(1.0, -1.0)
    local height = playdate.display.getHeight()
    local width = playdate.display.getWidth()
    local xStep = (stop.re - start.re) / width
    local yStep = (stop.im - start.im) / height
    for y = 0, height do
        for x = 0, width do
            local c = createComplex(start.re + x * xStep, start.im + y * yStep)
            local z = createComplex(0, 0)

            local iterations = 0
            while iterations < MAX_ITERATIONS do
                z = z:square():plus(c)

                if z.re * z.re + z.im * z.im > 4 then
                    break
                end
                iterations += 1
            end

            if iterations == MAX_ITERATIONS then
                gfx.drawPixel(x, y)
            end
        end
    end
end

function drawMandelbrotNoTables()
    gfx.setColor(gfx.kColorBlack)

    local startx = -2.5
    local starty = 1.0
    local stopx = 1.0
    local stopy = -1.0
    local height = playdate.display.getHeight()
    local width = playdate.display.getWidth()
    local xStep = (stopx - startx) / width
    local yStep = (stopy - starty) / height
    for y = 0, height do
        for x = 0, width do
            local cre = startx + x * xStep
            local cim = starty + y * yStep
            local zre = 0.0
            local zim = 0.0

            local iterations = 0
            while iterations < MAX_ITERATIONS do
                local old_zre = zre
                zre = (zre * zre - zim * zim) + cre
                zim = (2.0 * old_zre * zim) + cim

                if zre * zre + zim * zim > 4 then
                    break
                end
                iterations += 1
            end

            if iterations == MAX_ITERATIONS then
                gfx.drawPixel(x, y)
            end
        end
    end
end

hasRendered = false
function playdate.update()
    if hasRendered then
        return
    end
    hasRendered = true


    gfx.clear()
    playdate.resetElapsedTime()
    drawNativeMandelbrot()
    print(playdate.getElapsedTime())
    playdate.drawFPS(0, 0)
end

function playdate.AButtonDown()
    hasRendered = false
end
