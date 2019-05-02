/*

   Copyright 2018 Jeffrey Hullekes

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 */

let Flowify = {}

Flowify.canvas = function () {

    let my = {}

    my.canvasElement = document.getElementById("canvas")
    my.context2d = my.canvasElement.getContext("2d")
    
    // FIXME: we need a proper way of drawing shadows
    my.drawShadows = false
    my.nrOfDrawCalls = 0
    my.showNrOfDrawCalls = false
    
    my.usingPhysicalPixels = false
    my.scale = 1

    my.fixedCanvasSize = null

    my.isIE = false

    // NOTE: always keep this in sync with FontFamily in generic.h!
    my.fontFamilies = [
        "Arial",        // 0x00
        "Courier New",  // 0x01
    ]

    // Check if IE
    let ua = window.navigator.userAgent;
    if (ua.indexOf('MSIE ') > 0 || ua.indexOf('Trident/') > 0 || ua.indexOf('Edge/') > 0) {
        my.isIE = true
    }

    let requestAnimFrame = function () {
        return  window.requestAnimationFrame ||
            window.webkitRequestAnimationFrame   ||
            window.mozRequestAnimationFrame      ||
            window.oRequestAnimationFrame        ||
            window.msRequestAnimationFrame       ||
            function(callback, element){
                window.setTimeout(function(){

                    callback(+new Date)
                }, 1000 / 60)
            }
    }()

    // Taken from here: https://stackoverflow.com/questions/9677985/uncaught-typeerror-illegal-invocation-in-chrome
    my.requestAnimFrame = requestAnimFrame ? requestAnimFrame.bind(window) : null

    // (Re)sizing Canvas
    
    my.loadUsingPhysicalPixelsFromCookie = function () {
        my.usingPhysicalPixels = Flowify.input.getCookie('usingPhysicalPixels') === '1' ? true : false
        my.resizeCanvasToWindowSize()
    }

    my.resizeCanvas = function () {
        if (my.fixedCanvasSize == null) {
            my.resizeCanvasToWindowSize()
        }
        else {
            my.resizeCanvasToCustomSize(my.fixedCanvasSize)
        }
    }

    my.resizeCanvasToWindowSize = function () {
        
        // FIXME: when we change the *scale* of the canvas (by for example clicking on a button to re-scale), 
        //        the old mouse positions are now wrong. The problem is that we do not get any mouse-event
        //        when we resize/rescale the canvas! We should somehow get (and store) the mouse positions!
        
        let scale = 1
        if (my.usingPhysicalPixels) {
            if (window.devicePixelRatio) {
                // TODO: if we enable this, we should also respond to it by
                //       using bigger fonts and bigger shapes. For now this
                //       is turned off.
                scale = window.devicePixelRatio
            }
        }
        my.scale = scale
        
        if ( my.canvasElement.width != window.innerWidth * my.scale || my.canvasElement.height != window.innerHeight * my.scale) {
            my.canvasElement.style.width = window.innerWidth
            my.canvasElement.style.height = window.innerHeight
            my.canvasElement.width = window.innerWidth * my.scale
            my.canvasElement.height = window.innerHeight * my.scale
            // my.context2d.scale(my.scale, my.scale);  // apparently, this is not need. Maybe because it "scales to fit" somehow?
        }
        
    }

    my.resizeCanvasToCustomSize = function (customSize) {
        if ( my.canvasElement.width != customSize.width || my.canvasElement.height != customSize.height) {
            my.canvasElement.width = customSize.width
            my.canvasElement.height = customSize.height
        }
    }

    my.clearCanvas = function () {
        my.context2d.clearRect(0, 0, my.canvasElement.width, my.canvasElement.height)
        my.context2d.beginPath() // See: http://codetheory.in/why-clearrect-might-not-be-clearing-canvas-pixels/
        my.context2d.closePath()
        
        if (my.showNrOfDrawCalls) {
            my.context2d.font = "14px Arial"
            my.context2d.fillStyle = "black"
            my.context2d.textBaseline = "top"
            my.context2d.fillText(my.nrOfDrawCalls, 0, 0)
        }
        my.nrOfDrawCalls = 0
        
    }
        
    my.getCanvasRGBAColor = function (colorRGB, colorAlpha) {
        
        let red = colorRGB % 256
        colorRGB = (colorRGB - red) / 256
        let green = colorRGB % 256
        colorRGB = (colorRGB - green) / 256
        let blue = colorRGB % 256
        
        return "rgba(" + red + "," + green + "," + blue + "," + colorAlpha/255 + ")"
    }
    
    my.getExportedFunctions = function () {
        
        let ctx = my.context2d
        
        let exportedFunctions = {
            
            _jsClipRect: function (x, y, width, height) {
                ctx.save()
                
                ctx.beginPath()
                ctx.rect(x, y, width, height)
                ctx.clip()
                my.nrOfDrawCalls++
            },
            
            _jsUnClipRect: function () {
                ctx.restore()
                my.nrOfDrawCalls++
            },
            
            _jsDrawRoundedRect: function (x, y, width, height, r, lineColorRGB, lineColorAlpha, fillColorRGB, fillColorAlpha, lineWidth) {
                ctx.beginPath()     
                ctx.moveTo(x + r, y)
                ctx.arcTo(x + width, y, x + width, y + height, r)
                ctx.arcTo(x + width, y + height, x, y + height, r)
                ctx.arcTo(x, y + height, x, y, r)
                ctx.arcTo(x, y, x + width, y, r)
                ctx.closePath()

                if (my.drawShadows) {
                    ctx.shadowOffsetX = 3
                    ctx.shadowOffsetY = 3
                    ctx.shadowBlur    = 7
                }
                if (lineColorAlpha) {
                    if (my.drawShadows) {
                        ctx.shadowColor   = "#222222"
                        ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                        ctx.lineWidth = lineWidth
                        ctx.stroke()
                        my.nrOfDrawCalls++
                    }
                }
                
                if (fillColorAlpha) {
                    if (my.drawShadows) {
                        ctx.shadowColor = "transparent";
                    }
                    ctx.fillStyle = my.getCanvasRGBAColor(fillColorRGB, fillColorAlpha)
                    ctx.fill()
                    my.nrOfDrawCalls++
                }

                if (lineColorAlpha) {
                    if (my.drawShadows) {
                        ctx.shadowColor = "transparent";
                    }
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    my.nrOfDrawCalls++
                }
            },
            
            _jsDrawLanePaths: function (leftPathPartsIndex, leftPathPartsCount, rightPathPartsIndex, rightPathPartsCount, 
                                        lineColorRGB, lineColorAlpha, fillColorRGB, fillColorAlpha, lineWidth) {
                                       
                                       
                let leftPathParts = []
                let rightPathParts = []
                
                let PathPart_Move = 0
                let PathPart_Line = 1
                let PathPart_LineWhenBackground = 2
                let PathPart_Arc_DownToLeft = 3
                let PathPart_Arc_DownToRight = 4
                let PathPart_Arc_LeftToUp = 5
                let PathPart_Arc_LeftToDown = 6
                let PathPart_Arc_UpToLeft = 7
                let PathPart_Arc_UpToRight = 8
                let PathPart_Arc_RightToUp = 9
                let PathPart_Arc_RightToDown = 10
                
                let nrOfIntegersPerPathPart = 3 // FIXME: somehow sync this with the size of DrawablePathPart?
                
                for (let i = leftPathPartsIndex / 4; i < leftPathPartsIndex / 4 + leftPathPartsCount * nrOfIntegersPerPathPart; i = i + nrOfIntegersPerPathPart) {
                    let x = Flowify.main.bufferI32[i]
                    let y = Flowify.main.bufferI32[i+1]
                    let type = Flowify.main.bufferI32[i+2]
                    
                    leftPathParts.push({
                        "x": x,
                        "y": y,
                        "type": type,
                    })
                }
                
                for (let i = rightPathPartsIndex / 4; i < rightPathPartsIndex / 4 + rightPathPartsCount * nrOfIntegersPerPathPart; i = i + nrOfIntegersPerPathPart) {
                    let x = Flowify.main.bufferI32[i]
                    let y = Flowify.main.bufferI32[i+1]
                    let type = Flowify.main.bufferI32[i+2]
                    
                    rightPathParts.push({
                        "x": x,
                        "y": y,
                        "type": type,
                    })
                }

                // Drawing the lane as left and right paths
                
                drawPath = function (path, isBackground) {
                    let previousX = null
                    let previousY = null
                    for (let partIndex = 0; partIndex < path.length; partIndex++) {
                        let part = path[partIndex]
                        let type = part.type
                        let x = part.x
                        let y = part.y
                        
                        let radius = previousX - x
                        if (radius < 0) {
                            radius = -radius
                        }
                            
                        if (type === PathPart_Move) {
                            ctx.moveTo(x, y)
                        }
                        else if (type === PathPart_Line) {
                            ctx.lineTo(x, y)
                        }
                        else if (type === PathPart_LineWhenBackground) {
                            if (isBackground) {
                                ctx.lineTo(x, y)
                            }
                            else {
                                ctx.moveTo(x, y)
                            }
                        }
                        else if (type === PathPart_Arc_DownToLeft || type === PathPart_Arc_DownToRight || type === PathPart_Arc_UpToLeft || type === PathPart_Arc_UpToRight) {
                            ctx.arcTo(previousX, y, x, y, radius)
                        }
                        else if (type === PathPart_Arc_LeftToUp || type === PathPart_Arc_LeftToDown || type === PathPart_Arc_RightToUp || type === PathPart_Arc_RightToDown) {
                            ctx.arcTo(x, previousY, x, y, radius)
                        }
                                    
                        previousX = x
                        previousY = y
                    }
                }


                if (fillColorAlpha) {
                    ctx.beginPath()
                    drawPath(leftPathParts, isBackground = true)
                    drawPath(rightPathParts, isBackground = true)
                    ctx.closePath()
                    ctx.fillStyle = my.getCanvasRGBAColor(fillColorRGB, fillColorAlpha)
                    ctx.fill()
                    
                    my.nrOfDrawCalls++
                }

                if (lineColorAlpha) {
                    ctx.beginPath()
                    drawPath(leftPathParts, isBackground = false)
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    
                    my.nrOfDrawCalls++
                                    
                    ctx.beginPath()
                    drawPath(rightPathParts, isBackground = false)
                    ctx.strokeStyle =  my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    
                    my.nrOfDrawCalls++
                }
            },
            
            _jsDrawLane: function (lanePartsIndex, lanePartsCount,
                                   partialRectAtStart, isRightSideAtStart,
                                   partialRectAtEnd, isRightSideAtEnd,
                                   radius, lineColorRGB, lineColorAlpha, fillColorRGB, fillColorAlpha, lineWidth) {
                
                let Direction_TopToBottom = 0
                let Direction_LeftToRight = 1
                let Direction_BottomToTop = 2
                let Direction_RightToLeft = 3

                let laneParts = []
                let nrOfIntegersPerLanePart = 5 // FIXME: somehow sync this with the size of DirectionalRect2d?
                for (let i = lanePartsIndex / 4; i < lanePartsIndex / 4 + lanePartsCount * nrOfIntegersPerLanePart; i = i + nrOfIntegersPerLanePart) {
                    let x = Flowify.main.bufferI32[i]
                    let y = Flowify.main.bufferI32[i+1]
                    let width = Flowify.main.bufferI32[i+2]
                    let height = Flowify.main.bufferI32[i+3]
                    let direction = Flowify.main.bufferI32[i+4]
                    
                    laneParts.push({
                        "x": x,
                        "y": y,
                        "width": width,
                        "height": height,
                        "direction": direction,
                    })
                }
                
                if (laneParts.length <= 0) {
                    return
                }
                
                let DrawMove = 0
                let DrawLine = 1
                let DrawLineWhenBackground = 2
                let DrawArc_DownToLeft = 3
                let DrawArc_DownToRight = 4
                let DrawArc_LeftToUp = 5
                let DrawArc_LeftToDown = 6
                let DrawArc_UpToLeft = 7
                let DrawArc_UpToRight = 8
                let DrawArc_RightToUp = 9
                let DrawArc_RightToDown = 10
                
                let leftPath = []
                let rightPath = []
                
                function addPart(x, y, type, isRight) {
                    if (isRight) {
                        rightPath.push({"type": type, "x": x, "y": y})
                    }
                    else {
                        leftPath.push({"type": type, "x": x, "y": y})
                    }
                }
                
                function drawLeft(leftTopX, topY, leftMiddleY, leftBottomX, bottomY, radius, skipTopPartWhenBorder = false, skipBottomPartWhenBorder = false) {

                    let topLineType = DrawLine
                    if (skipTopPartWhenBorder) {
                        topLineType = DrawLineWhenBackground
                    }
                    let bottomLineType = DrawLine
                    if (skipBottomPartWhenBorder) {
                        bottomLineType = DrawLineWhenBackground
                    }
                    
                    // Draw left side
                    // TODO: we should not do this!?
                    ctx.lineTo(leftTopX, topY)
                        if (!leftPath.length) addPart(leftTopX, topY, DrawLineWhenBackground, false)
                    
                    if (leftBottomX < leftTopX) {
                        // bottom is to the left of the top
                        if (leftTopX - leftBottomX < radius * 2) {
                            radius = (leftTopX - leftBottomX) / 2
                        }
                        addPart(leftTopX, leftMiddleY - radius, topLineType, false)
                        addPart(leftTopX - radius, leftMiddleY, DrawArc_DownToLeft, false)
                        addPart(leftBottomX + radius, leftMiddleY, DrawLine, false)
                        addPart(leftBottomX, leftMiddleY + radius, DrawArc_LeftToDown, false)
                        addPart(leftBottomX, bottomY, bottomLineType, false)
                    }
                    else if (leftBottomX > leftTopX) {
                        // bottom is to the right of the top
                        if (leftBottomX - leftTopX < radius * 2) {
                            radius = (leftBottomX - leftTopX) / 2
                        }
                        addPart(leftTopX, leftMiddleY - radius, topLineType, false)
                        addPart(leftTopX + radius, leftMiddleY, DrawArc_DownToRight, false)
                        addPart(leftBottomX - radius, leftMiddleY, DrawLine, false)
                        addPart(leftBottomX, leftMiddleY + radius, DrawArc_RightToDown, false)
                        addPart(leftBottomX, bottomY, bottomLineType, false)
                    }
                    else {
                        // straight vertical line
                        addPart(leftBottomX, bottomY, DrawLine, false)
                    }
                }
                
                function drawRight(rightTopX, topY, rightMiddleY, rightBottomX, bottomY, radius, skipTopPartWhenBorder = false, skipBottomPartWhenBorder = false) {
                    
                    let topLineType = DrawLine
                    if (skipTopPartWhenBorder) {
                        topLineType = DrawLineWhenBackground
                    }
                    let bottomLineType = DrawLine
                    if (skipBottomPartWhenBorder) {
                        bottomLineType = DrawLineWhenBackground
                    }
                    
                    // Right side (bottom to top)
                    // TODO: we should not do this!?
                    ctx.lineTo(rightBottomX, bottomY)
                        if (!rightPath.length) addPart(rightBottomX, bottomY, DrawLineWhenBackground, true)
                            
                    if (rightBottomX < rightTopX) {
                        // bottom is to the left of the top
                        if (rightTopX - rightBottomX < radius * 2) {
                            radius = (rightTopX - rightBottomX) / 2
                        }
                        addPart(rightBottomX, rightMiddleY + radius, bottomLineType, true)
                        addPart(rightBottomX + radius, rightMiddleY, DrawArc_UpToRight, true)
                        addPart(rightTopX - radius, rightMiddleY, DrawLine, true)
                        addPart(rightTopX, rightMiddleY - radius, DrawArc_RightToUp, true)
                        addPart(rightTopX, topY, topLineType, true)
                    }
                    else if (rightBottomX > rightTopX) {
                        // bottom is to the right of the top
                        if (rightBottomX - rightTopX < radius * 2) {
                            radius = (rightBottomX - rightTopX) / 2
                        }
                        addPart(rightBottomX, rightMiddleY + radius, bottomLineType, true)
                        addPart(rightBottomX - radius, rightMiddleY, DrawArc_UpToLeft, true)
                        addPart(rightTopX + radius, rightMiddleY, DrawLine, true)
                        addPart(rightTopX, rightMiddleY - radius, DrawArc_LeftToUp, true)
                        addPart(rightTopX, topY, topLineType, true)
                    }
                    else {
                        // straight vertical line
                        ctx.lineTo(rightTopX, topY)
                            addPart(rightTopX, topY, DrawLine, true)
                    }
                }
                
                function drawOneSideOfCorner(firstX, firstY, firstDirection, secondX, secondY, secondDirection, radius, isRight, skipTopPartWhenBorder = false, skipBottomPartWhenBorder = false) {
                    
                    if (!isRight && !leftPath.length) addPart(firstX, firstY, DrawLineWhenBackground, isRight)
                    if (isRight && !rightPath.length) addPart(firstX, firstY, DrawLineWhenBackground, isRight)
                
                    // FIXME: adjust radius to make it fit!
                    
                    // Forwards
                    if (firstDirection == Direction_TopToBottom && secondDirection == Direction_LeftToRight) {
                        addPart(firstX, secondY - radius, DrawLine, isRight)
                        addPart(firstX + radius, secondY, DrawArc_DownToRight, isRight)
                        addPart(secondX, secondY, DrawLine, isRight)
                    }
                    else if (firstDirection == Direction_LeftToRight && secondDirection == Direction_BottomToTop) {
                        addPart(secondX - radius, firstY, DrawLine, isRight)
                        addPart(secondX, firstY - radius, DrawArc_RightToUp, isRight)
                        addPart(secondX, secondY, DrawLine, isRight)
                    }
                    else if (firstDirection == Direction_BottomToTop && secondDirection == Direction_RightToLeft) {
                        addPart(firstX, secondY + radius, DrawLine, isRight)
                        addPart(firstX - radius, secondY, DrawArc_UpToLeft, isRight)
                        addPart(secondX, secondY, DrawLine, isRight)
                    }
                    else if (firstDirection == Direction_RightToLeft && secondDirection == Direction_TopToBottom) {
                        addPart(secondX + radius, firstY, DrawLine, isRight)
                        addPart(secondX, firstY + radius, DrawArc_LeftToDown, isRight)
                        addPart(secondX, secondY, DrawLine, isRight)
                    }
                    // Backwards
                    else if (firstDirection == Direction_TopToBottom && secondDirection == Direction_RightToLeft) {
                        addPart(firstX, secondY + radius, DrawLine, isRight)
                        addPart(firstX + radius, secondY, DrawArc_UpToRight, isRight)
                        addPart(secondX, secondY, DrawLine, isRight)
                    }
                    else if (firstDirection == Direction_RightToLeft && secondDirection == Direction_BottomToTop) {
                        addPart(secondX - radius, firstY, DrawLine, isRight)
                        addPart(secondX, firstY + radius, DrawArc_RightToDown, isRight)
                        addPart(secondX, secondY, DrawLine, isRight)
                    }
                    else if (firstDirection == Direction_BottomToTop && secondDirection == Direction_LeftToRight) {
                        addPart(firstX, secondY - radius, DrawLine, isRight)
                        addPart(firstX - radius, secondY, DrawArc_DownToLeft, isRight)
                        addPart(secondX, secondY, DrawLine, isRight)
                    }
                    else if (firstDirection == Direction_LeftToRight && secondDirection == Direction_TopToBottom) {
                        addPart(secondX + radius, firstY, DrawLine, isRight)
                        addPart(secondX, firstY - radius, DrawArc_LeftToUp, isRight)
                        addPart(secondX, secondY, DrawLine, isRight)
                    }
                    else {
                        console.log("ERROR: unsupported combination of directions!")
                    }
                }
                
                
                // Create left and right paths
                
                // Left side (top to bottom)
                
                if (!partialRectAtStart) {
                    let lanePart = laneParts[0]
                    
                    // args: leftTopX, topY, leftMiddleY, leftBottomX, bottomY, radius
                    drawLeft(lanePart.x, lanePart.y, 
                            lanePart.y + lanePart.height / 4, // TODO: 1 / 4 as middleY (we shouldnt use drawLeft, we only draw a straight (half) lanePart)
                            lanePart.x, lanePart.y + lanePart.height / 2, radius)
                }
                
                let previousLanePart = null
                for (let lanePartIndex = 0; lanePartIndex < laneParts.length; lanePartIndex++) {
                    let lanePart = laneParts[lanePartIndex]
                    
                    if (lanePartIndex > 0) {
                        
                        if (lanePart.direction != Direction_TopToBottom || previousLanePart.direction != Direction_TopToBottom) {
                            let firstX = previousLanePart.x
                            let firstY = previousLanePart.y
                            let firstDirection = previousLanePart.direction
                            
                            if (firstDirection == Direction_LeftToRight) {
                                firstY = previousLanePart.y + previousLanePart.height
                            }
                            else if (firstDirection == Direction_BottomToTop) {
                                firstX = previousLanePart.x + previousLanePart.width
                            }
                            
                            let secondX = lanePart.x
                            let secondY = lanePart.y
                            let secondDirection = lanePart.direction
                            
                            if (secondDirection == Direction_BottomToTop) {
                                secondX = lanePart.x + lanePart.width
                            }
                            else if (secondDirection == Direction_LeftToRight) {
                                secondY = lanePart.y + lanePart.height
                            }
                        
                            drawOneSideOfCorner(firstX, firstY, firstDirection, secondX, secondY, secondDirection, radius, isRight = false)
                        }
                        else {
                        
                            let skipTopPartWhenBorder = false
                            let skipBottomPartWhenBorder = false
                            
                            // TODO: where do we want the leftMiddleY to be?
                            let distanceBetweenRects = lanePart.y - (previousLanePart.y + previousLanePart.height)
                            let leftMiddleY = lanePart.y - distanceBetweenRects / 2
                            
                            let heightToDrawOfPreviousLane = previousLanePart.height / 2
                            let heightToDrawOfCurrentLane = lanePart.height / 2
                        
                            // We are at the beginning of a lane that begins from a splitter (its right side)
                            // We should not *stroke* the left side of the previous lanePart,
                            // which is the last part of the previous lane: a splitter.
                            if (lanePartIndex == 1 && partialRectAtStart && isRightSideAtStart) {
                                // TODO: now forcing leftMiddleY to be right below the top lanePart. We might want to add an extra line/move instead (towards a real splitting point)
                                leftMiddleY = previousLanePart.y + previousLanePart.height + radius
                                skipTopPartWhenBorder = true
                            }
                            
                            // We are at the end of a lane that ends in a joiner (its right side)
                            // We should not *stroke* the left side of the current lanePart,
                            // which is the first part of the next lane: a joiner.
                            if (lanePartIndex == laneParts.length - 1 && partialRectAtEnd && isRightSideAtEnd) {
                                // TODO: now forcing leftMiddleY to be right above the bottom lanePart. We might want to add an extra line/move instead (towards a real joining point)
                                leftMiddleY = lanePart.y - radius
                                skipBottomPartWhenBorder = true
                            }
                            
                            // args: leftTopX, topY, leftMiddleY, leftBottomX, bottomY, radius
                            drawLeft(previousLanePart.x, previousLanePart.y + previousLanePart.height - heightToDrawOfPreviousLane, 
                                    leftMiddleY,
                                    lanePart.x, lanePart.y + heightToDrawOfCurrentLane, radius, skipTopPartWhenBorder, skipBottomPartWhenBorder)
                        }
                        
                    }
                    else {
                        // the case lanePartIndex == 0 is handled above
                    }
                    
                    previousLanePart = lanePart
                }
                    
                if (!partialRectAtEnd) {
                    let lanePart = laneParts[laneParts.length - 1]
                    
                    // args: leftTopX, topY, leftMiddleY, leftBottomX, bottomY, radius
                    drawLeft(lanePart.x, lanePart.y + lanePart.height / 2, 
                            lanePart.y + lanePart.height * 3 / 4, // TODO: 3 / 4 as middleY (we shouldnt use drawLeft, we only draw a straight (half) lanePart)
                            lanePart.x, lanePart.y + lanePart.height, radius)
                }
                
                // Right side (bottom to top)
                if (!partialRectAtEnd) {
                    let lanePart = laneParts[laneParts.length - 1]
                    
                    // args: rightTopX, topY, rightMiddleY, rightBottomX, bottomY, radius
                    drawRight(lanePart.x + lanePart.width, lanePart.y + lanePart.height / 2, 
                              lanePart.y + lanePart.height * 3 / 4, // TODO: 3 / 4 as middleY (we shouldnt use drawLeft, we only draw a straight (half) lanePart)
                              lanePart.x + lanePart.width, lanePart.y + lanePart.height, radius)
                }
                
                previousLanePart = null
                for (let lanePartIndex = laneParts.length - 1; lanePartIndex >= 0; lanePartIndex--) {
                    let lanePart = laneParts[lanePartIndex]
                    
                    if (lanePartIndex < laneParts.length - 1) {
                        
                        if (lanePart.direction != Direction_TopToBottom || previousLanePart.direction != Direction_TopToBottom) {
                            let firstX = previousLanePart.x
                            let firstY = previousLanePart.y
                            let firstDirection = previousLanePart.direction
                            
                            if (firstDirection == Direction_RightToLeft) {
                                firstY = previousLanePart.y + previousLanePart.height
                            }
                            else if (firstDirection == Direction_TopToBottom) {
                                firstX = previousLanePart.x + previousLanePart.width
                            }
                            
                            let secondX = lanePart.x
                            let secondY = lanePart.y
                            let secondDirection = lanePart.direction
                            
                            if (secondDirection == Direction_TopToBottom) {
                                secondX = lanePart.x + lanePart.width
                            }
                            else if (secondDirection == Direction_RightToLeft) {
                                secondY = lanePart.y + lanePart.height
                            }
                        
                            drawOneSideOfCorner(firstX, firstY, firstDirection, secondX, secondY, secondDirection, radius, isRight = true)
                        }
                        else {
                        
                            let skipTopPartWhenBorder = false
                            let skipBottomPartWhenBorder = false
                            
                            // TODO: where do we want the rightMiddleY to be?
                            let distanceBetweenRects = previousLanePart.y - (lanePart.y + lanePart.height)
                            let rightMiddleY = previousLanePart.y - distanceBetweenRects / 2
                            
                            let heightToDrawOfPreviousLane = previousLanePart.height / 2
                            let heightToDrawOfCurrentLane = lanePart.height / 2
                            
                            // We are at the beginning of a lane that begins from a splitter (its left side)
                            // We should not *stroke* the right side of the previous lanePart,
                            // which is the last part of the previous lane: a splitter.
                            if (lanePartIndex == 0 && partialRectAtStart && !isRightSideAtStart) {
                                // TODO: now forcing rightMiddleY to be right below the top lanePart. We might want to add an extra line/move instead (towards a real splitting point)
                                rightMiddleY = lanePart.y + lanePart.height + radius
                                skipTopPartWhenBorder = true
                            }
                            
                            // We are at the end of a lane that ends in a joiner (its left side)
                            // We should not *stroke* the right side of the current lanePart,
                            // which is the first part of the next lane: a joiner.
                            if (lanePartIndex == laneParts.length - 2 && partialRectAtEnd && !isRightSideAtEnd) {
                                // TODO: now forcing rightMiddleY to be right above the bottom lanePart. We might want to add an extra line/move instead (towards a real joining point)
                                rightMiddleY = previousLanePart.y - radius
                                skipBottomPartWhenBorder = true
                            }
                            
                            // args: rightTopX, topY, rightMiddleY, rightBottomX, bottomY, radius
                            drawRight(lanePart.x + lanePart.width, lanePart.y + lanePart.height - heightToDrawOfCurrentLane, 
                                      rightMiddleY,
                                      previousLanePart.x + previousLanePart.width, previousLanePart.y + heightToDrawOfPreviousLane, radius, skipTopPartWhenBorder, skipBottomPartWhenBorder)
                        }
                    }
                    else {
                        // the case laneParts.length - 1 is handled above
                    }
                                
                    previousLanePart = lanePart
                }
                
                if (!partialRectAtStart) {
                    let lanePart = laneParts[0]
                    
                    // args: rightTopX, topY, rightMiddleY, rightBottomX, bottomY, radius
                    drawRight(lanePart.x + lanePart.width, lanePart.y, 
                              lanePart.y + lanePart.height / 4, // TODO: 1 / 4 as middleY (we shouldnt use drawLeft, we only draw a straight (half) lanePart)
                              lanePart.x + previousLanePart.width, lanePart.y + lanePart.height / 2, radius)
                }
                
                
                

                // Drawing the lane as left and right paths
                
                drawPath = function (path, isBackground) {
                    let previousX = null
                    let previousY = null
                    for (let partIndex = 0; partIndex < path.length; partIndex++) {
                        let part = path[partIndex]
                        let type = part.type
                        let x = part.x
                        let y = part.y
                        
                        let radius = previousX - x
                        if (radius < 0) {
                            radius = -radius
                        }
                            
                        if (type === DrawMove) {
                            ctx.moveTo(x, y)
                        }
                        else if (type === DrawLine) {
                            ctx.lineTo(x, y)
                        }
                        else if (type === DrawLineWhenBackground) {
                            if (isBackground) {
                                ctx.lineTo(x, y)
                            }
                            else {
                                ctx.moveTo(x, y)
                            }
                        }
                        else if (type === DrawArc_DownToLeft || type === DrawArc_DownToRight || type === DrawArc_UpToLeft || type === DrawArc_UpToRight) {
                            ctx.arcTo(previousX, y, x, y, radius)
                        }
                        else if (type === DrawArc_LeftToUp || type === DrawArc_LeftToDown || type === DrawArc_RightToUp || type === DrawArc_RightToDown) {
                            ctx.arcTo(x, previousY, x, y, radius)
                        }
                                    
                        previousX = x
                        previousY = y
                    }
                }


                if (fillColorAlpha) {
                    ctx.beginPath()
                    drawPath(leftPath, isBackground = true)
                    drawPath(rightPath, isBackground = true)
                    ctx.closePath()
                    ctx.fillStyle = my.getCanvasRGBAColor(fillColorRGB, fillColorAlpha)
                    ctx.fill()
                    
                    my.nrOfDrawCalls++
                }

                if (lineColorAlpha) {
                    ctx.beginPath()
                    drawPath(leftPath, isBackground = false)
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    
                    my.nrOfDrawCalls++
                                    
                    ctx.beginPath()
                    drawPath(rightPath, isBackground = false)
                    ctx.strokeStyle =  my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    
                    my.nrOfDrawCalls++
                }

            },
            
            _jsDrawRect: function (x, y, width, height, lineColorRGB, lineColorAlpha, fillColorRGB, fillColorAlpha, lineWidth) {

                ctx.beginPath()
                ctx.rect(x, y, width, height)

                if (fillColorAlpha) {
                    ctx.fillStyle = my.getCanvasRGBAColor(fillColorRGB, fillColorAlpha)
                    ctx.fill()
                    my.nrOfDrawCalls++
                }

                if (lineColorAlpha) {
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    my.nrOfDrawCalls++
                }
            },
            
            _jsDrawLine: function (xStart, yStart, xEnd, yEnd, lineColorRGB, lineColorAlpha, lineWidth, roundCap) {

                ctx.beginPath()
                ctx.moveTo(xStart, yStart)
                ctx.lineTo(xEnd, yEnd)
                if (roundCap) {
                    ctx.lineCap = "round"
                }

                if (lineColorAlpha) {
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    my.nrOfDrawCalls++
                }
            },
            
            _jsDrawEllipse: function (x, y, width, height, lineColorRGB, lineColorAlpha, fillColorRGB, fillColorAlpha, lineWidth) {
                
                function drawEllipseIE(x, y, w, h) {
                    
                    // This is from: http://stackoverflow.com/questions/2172798/how-to-draw-an-oval-in-html5-canvas
                    var kappa = .5522848,
                        ox = (w / 2) * kappa, // control point offset horizontal
                        oy = (h / 2) * kappa, // control point offset vertical
                        xe = x + w,           // x-end
                        ye = y + h,           // y-end
                        xm = x + w / 2,       // x-middle
                        ym = y + h / 2;       // y-middle

                    ctx.beginPath();
                    ctx.moveTo(x, ym);
                    ctx.bezierCurveTo(x, ym - oy, xm - ox, y, xm, y);
                    ctx.bezierCurveTo(xm + ox, y, xe, ym - oy, xe, ym);
                    ctx.bezierCurveTo(xe, ym + oy, xm + ox, ye, xm, ye);
                    ctx.bezierCurveTo(xm - ox, ye, x, ym + oy, x, ym);
                    // ctx.stroke();  not doing this here
                }
                
                ctx.beginPath()
                if (my.isIE) {
                    drawEllipseIE(x, y, width, height)
                }
                else {
                    ctx.ellipse(x + width / 2, y + height / 2, width / 2, height / 2, 0, 0, 2 * Math.PI);
                }

                if (fillColorAlpha) {
                    ctx.fillStyle = my.getCanvasRGBAColor(fillColorRGB, fillColorAlpha)
                    ctx.fill()
                    my.nrOfDrawCalls++
                }
                if (lineColorAlpha) {
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    my.nrOfDrawCalls++
                }
            },
            
            _jsDrawText: function (x, y, stringIndex, stringLength, fontHeight, fontFamilyIndex, fontColorRGB, fontColorAlpha) { // , baseFontIndex, baseFontLength) {
                let string = ""
                for (let i = stringIndex; i < stringIndex + stringLength; i++) {
                    string += String.fromCharCode(Flowify.main.bufferU8[i])
                }
                
                // TODO: check if out-of-bounds
                let baseFont = my.fontFamilies[fontFamilyIndex]

                // Note: fontHeight is the distance between the top of a capital and the bottom of a capital (note that a non-capital can stick out below!)
                ctx.font = fontHeight + "px " + baseFont
                let fontHeightPx = fontHeight
                ctx.fillStyle = my.getCanvasRGBAColor(fontColorRGB, fontColorAlpha)
                ctx.textBaseline = "top"
                ctx.fillText(string, x, y)
                my.nrOfDrawCalls++
            },

            _jsGetTextWidth: function (stringIndex, stringLength, fontHeight, fontFamilyIndex) {

                let string = ""
                for (let i = stringIndex; i < stringIndex + stringLength; i++) {
                    string += String.fromCharCode(Flowify.main.bufferU8[i])
                }
                // TODO: check if out-of-bounds
                let baseFont = my.fontFamilies[fontFamilyIndex]

                // Note: fontHeight is the distance between the top of a capital and the bottom of a capital (note that a non-capital can stick out below!)
                ctx.font = fontHeight + "px " + baseFont
                let fontHeightPx = fontHeight
                let textMetrics = ctx.measureText(string)
                my.nrOfDrawCalls++
                
                return textMetrics.width
            },

            _jsLog: function(stringIndex, stringLength) {
                let string = ""
                for (let i = stringIndex; i < stringIndex + stringLength; i++) {
                    string += String.fromCharCode(Flowify.main.bufferU8[i])
                }
                console.log(string)
            },
            
            _jsAbort: function(stringIndex, stringLength, fileNameIndex, fileNameLength, lineNumber) {
                let string = ""
                for (let i = stringIndex; i < stringIndex + stringLength; i++) {
                    string += String.fromCharCode(Flowify.main.bufferU8[i])
                }
                let fileName = ""
                for (let i = fileNameIndex; i < fileNameIndex + fileNameLength; i++) {
                    fileName += String.fromCharCode(Flowify.main.bufferU8[i])
                }
                let error = "Assertion failed: " + string + " in " + fileName + " at line " + lineNumber
                throw error
            },
        
            _jsSetUsingPhysicalPixels: function(usingPhysicalPixels) {
                my.usingPhysicalPixels = usingPhysicalPixels
                Flowify.input.storeCookie('usingPhysicalPixels', usingPhysicalPixels)
                
                // TODO: should we really do this here? We might still be in de middle of rendering. Better to do this after the render, right?
                my.resizeCanvasToWindowSize()
            }
            
            /*
            // TODO: do we want to use this method? (keeping the same bending radius, but not drawing up-to 90 degrees)
            
            let ctx = my.context2d
            
            let lineColorAlpha = 255
            let lineColorRGB = 0
            
            let r = 16 * 4
            let lineWidth = 2
            
            let leftTopX = 400
            //let rightTopX
            let topY = 400
            
            let leftBottomX = 500
            //let rightBottomX
            let bottomY = 500
            
            let leftMiddleY = 450
            // let rightMiddleY
            
            // FIXME: this assumes bottom to the right of the top!
            let halfHorizontalDistanceTopBottom = (leftBottomX - leftTopX) / 2
            // FIXME: this only works if halfHorizontalDistanceTopBottom < r !!
            let angle = Math.acos((r - halfHorizontalDistanceTopBottom) / r)
            let distanceFromTopToEndOfArc = Math.sin(angle) * r

            ctx.strokeStyle = "#0000DD"
            ctx.beginPath()
            ctx.moveTo(leftTopX, topY)
            ctx.lineTo(leftTopX, bottomY)
            ctx.stroke()
            
            ctx.beginPath()
            ctx.moveTo(leftTopX + halfHorizontalDistanceTopBottom, topY)
            ctx.lineTo(leftTopX + halfHorizontalDistanceTopBottom, bottomY)
            ctx.stroke()
            
            ctx.beginPath()
            ctx.moveTo(leftBottomX, topY)
            ctx.lineTo(leftBottomX, bottomY)
            ctx.stroke()
            
            ctx.beginPath()
            ctx.moveTo(leftTopX, topY + distanceFromTopToEndOfArc)
            ctx.lineTo(leftBottomX, topY + distanceFromTopToEndOfArc)
            ctx.stroke()
            
            
            let startAngle = Math.PI
            let endAngle = Math.PI - angle
            let antiClockwise = true
            ctx.beginPath()     
            ctx.moveTo(leftTopX, topY)
            ctx.arc(leftTopX + r, topY, r, startAngle, endAngle, antiClockwise)
            ctx.arc(leftBottomX - r, topY + distanceFromTopToEndOfArc * 2, r, 2*Math.PI - angle, 0, false)

            if (lineColorAlpha) {
                ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                ctx.lineWidth = lineWidth
                ctx.stroke()
                my.nrOfDrawCalls++
            }
            
            */
        }
        
        return exportedFunctions
    }

    return my
}()
