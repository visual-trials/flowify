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
    my.showNrOfDrawCalls = true
    
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
        
        
        // FIXME: put this inside a nice function
        
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
            
            _jsDrawCorneredLaneSegment: function (horLeftX, horRightX, horY, vertX, vertTopY, vertBottomY, radius, lineColorRGB, lineColorAlpha, fillColorRGB, fillColorAlpha, lineWidth) {
                
                if (my.drawShadows) {
                    ctx.shadowOffsetX = 3
                    ctx.shadowOffsetY = 3
                    ctx.shadowBlur    = 7
                    
                    if (lineColorAlpha) {
                        // Draw borders
                        
                        ctx.beginPath()     
                        
                        ctx.moveTo(horLeftX, horY)
                        if (vertX < horLeftX) {
                            // The lane ends to the left of the beginning (we are drawing West)
                            if (vertBottomY < horY) {
                                // The lane ends to the top of the beginning (we are drawing West -> North)
                                
                                ctx.arcTo(horLeftX, vertBottomY, horLeftX - radius, vertBottomY, radius)
                                ctx.lineTo(vertX, vertBottomY)
                                
                                ctx.moveTo(vertX, vertTopY)
                                
                                ctx.arcTo(horRightX, vertTopY, horRightX, vertTopY + radius, radius)
                            }
                            else {
                                // The lane ends to the bottom of the beginning (we are drawing West -> South)
                                
                                ctx.arcTo(horLeftX, vertTopY, horLeftX - radius, vertTopY, radius)
                                ctx.lineTo(vertX, vertTopY)
                                
                                ctx.moveTo(vertX, vertBottomY)
                                
                                ctx.arcTo(horRightX, vertBottomY, horRightX, vertBottomY - radius, radius)
                            }
                        }
                        else {
                            // The lane ends to the right of the beginning (we are drawing East)
                            if (vertBottomY < horY) {
                                // The lane ends to the top of the beginning (we are drawing East -> North)
                                
                                ctx.arcTo(horLeftX, vertTopY, horLeftX + radius, vertTopY, radius)
                                ctx.lineTo(vertX, vertTopY)
                                
                                ctx.moveTo(vertX, vertBottomY)
                                
                                ctx.arcTo(horRightX, vertBottomY, horRightX, vertBottomY + radius, radius)
                            }
                            else {
                                // The lane ends to the bottom of the beginning (we are drawing East -> South)
                                
                                ctx.arcTo(horLeftX, vertBottomY, horLeftX + radius, vertBottomY, radius)
                                ctx.lineTo(vertX, vertBottomY)
                                
                                ctx.moveTo(vertX, vertTopY)
                                
                                ctx.arcTo(horRightX, vertTopY, horRightX, vertTopY - radius, radius)
                            }
                        }
                        ctx.lineTo(horRightX, horY)
                        if (my.drawShadows) {
                            ctx.shadowColor   = "#222222"
                        }
                        ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                        ctx.lineWidth = lineWidth
                        ctx.stroke()
                        my.nrOfDrawCalls++
                    }
                }
                
                if (fillColorAlpha) {
                    // Draw background
                    
                    ctx.beginPath()
                    
                    ctx.moveTo(horLeftX, horY)
                    if (vertX < horLeftX) {
                        // The lane ends to the left of the beginning (we are drawing West)
                        if (vertBottomY < horY) {
                            // The lane ends to the top of the beginning (we are drawing West -> North)
                            
                            ctx.arcTo(horLeftX, vertBottomY, horLeftX - radius, vertBottomY, radius)
                            ctx.lineTo(vertX, vertBottomY)
                            
                            ctx.lineTo(vertX, vertTopY)
                            
                            ctx.arcTo(horRightX, vertTopY, horRightX, vertTopY + radius, radius)
                        }
                        else {
                            // The lane ends to the bottom of the beginning (we are drawing West -> South)
                            
                            ctx.arcTo(horLeftX, vertTopY, horLeftX - radius, vertTopY, radius)
                            ctx.lineTo(vertX, vertTopY)
                            
                            ctx.lineTo(vertX, vertBottomY)
                            
                            ctx.arcTo(horRightX, vertBottomY, horRightX, vertBottomY - radius, radius)
                        }
                    }
                    else {
                        // The lane ends to the right of the beginning (we are drawing East)
                        if (vertBottomY < horY) {
                            // The lane ends to the top of the beginning (we are drawing East -> North)
                            
                            ctx.arcTo(horLeftX, vertTopY, horLeftX + radius, vertTopY, radius)
                            ctx.lineTo(vertX, vertTopY)
                            
                            ctx.lineTo(vertX, vertBottomY)
                            
                            ctx.arcTo(horRightX, vertBottomY, horRightX, vertBottomY + radius, radius)
                        }
                        else {
                            // The lane ends to the bottom of the beginning (we are drawing East -> South)
                            
                            ctx.arcTo(horLeftX, vertBottomY, horLeftX + radius, vertBottomY, radius)
                            ctx.lineTo(vertX, vertBottomY)
                            
                            ctx.lineTo(vertX, vertTopY)
                            
                            ctx.arcTo(horRightX, vertTopY, horRightX, vertTopY - radius, radius)
                        }
                    }
                    ctx.lineTo(horRightX, horY)
                    ctx.closePath()
                
                    if (my.drawShadows) {
                        ctx.shadowColor = "transparent";
                    }
                    ctx.fillStyle = my.getCanvasRGBAColor(fillColorRGB, fillColorAlpha)
                    ctx.fill()
                    my.nrOfDrawCalls++
                }
                
                if (lineColorAlpha) {
                    // Draw borders
                    
                    ctx.beginPath()     
                    
                    ctx.moveTo(horLeftX, horY)
                    if (vertX < horLeftX) {
                        // The lane ends to the left of the beginning (we are drawing West)
                        if (vertBottomY < horY) {
                            // The lane ends to the top of the beginning (we are drawing West -> North)
                            
                            ctx.arcTo(horLeftX, vertBottomY, horLeftX - radius, vertBottomY, radius)
                            ctx.lineTo(vertX, vertBottomY)
                            
                            ctx.moveTo(vertX, vertTopY)
                            
                            ctx.arcTo(horRightX, vertTopY, horRightX, vertTopY + radius, radius)
                        }
                        else {
                            // The lane ends to the bottom of the beginning (we are drawing West -> South)
                            
                            ctx.arcTo(horLeftX, vertTopY, horLeftX - radius, vertTopY, radius)
                            ctx.lineTo(vertX, vertTopY)
                            
                            ctx.moveTo(vertX, vertBottomY)
                            
                            ctx.arcTo(horRightX, vertBottomY, horRightX, vertBottomY - radius, radius)
                        }
                    }
                    else {
                        // The lane ends to the right of the beginning (we are drawing East)
                        if (vertBottomY < horY) {
                            // The lane ends to the top of the beginning (we are drawing East -> North)
                            
                            ctx.arcTo(horLeftX, vertTopY, horLeftX + radius, vertTopY, radius)
                            ctx.lineTo(vertX, vertTopY)
                            
                            ctx.moveTo(vertX, vertBottomY)
                            
                            ctx.arcTo(horRightX, vertBottomY, horRightX, vertBottomY + radius, radius)
                        }
                        else {
                            // The lane ends to the bottom of the beginning (we are drawing East -> South)
                            
                            ctx.arcTo(horLeftX, vertBottomY, horLeftX + radius, vertBottomY, radius)
                            ctx.lineTo(vertX, vertBottomY)
                            
                            ctx.moveTo(vertX, vertTopY)
                            
                            ctx.arcTo(horRightX, vertTopY, horRightX, vertTopY - radius, radius)
                        }
                    }
                    ctx.lineTo(horRightX, horY)
                    if (my.drawShadows) {
                        ctx.shadowColor = "transparent";
                    }
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    my.nrOfDrawCalls++
                }
            },
            
            _jsDrawLaneSegment: function (leftTopX, rightTopX, topY, leftBottomX, rightBottomX, bottomY, leftMiddleY, rightMiddleY, radius, lineColorRGB, lineColorAlpha, fillColorRGB, fillColorAlpha, lineWidth) {
                
                if (my.drawShadows) {
                    ctx.shadowOffsetX = 3
                    ctx.shadowOffsetY = 3
                    ctx.shadowBlur    = 7
                    if (lineColorAlpha) {
                        // Draw left side
                        ctx.beginPath()     
                        ctx.moveTo(leftTopX, topY)
                        if (leftBottomX < leftTopX) {
                            // bottom is to the left of the top
                            ctx.arcTo(leftTopX, leftMiddleY, leftTopX - radius, leftMiddleY, radius)
                            ctx.arcTo(leftBottomX, leftMiddleY, leftBottomX, leftMiddleY + radius, radius)
                            ctx.lineTo(leftBottomX, bottomY)
                        }
                        else if (leftBottomX > leftTopX) {
                            // bottom is to the right of the top
                            ctx.arcTo(leftTopX, leftMiddleY, leftTopX + radius, leftMiddleY, radius)
                            ctx.arcTo(leftBottomX, leftMiddleY, leftBottomX, leftMiddleY + radius, radius)
                            ctx.lineTo(leftBottomX, bottomY)
                        }
                        else {
                            // straight vertical line
                            ctx.lineTo(leftBottomX, bottomY)
                        }
                        ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                        ctx.lineWidth = lineWidth
                        ctx.stroke()
                        
                        // Draw right side
                        
                        ctx.beginPath()     
                        ctx.moveTo(rightTopX, topY)
                        if (rightBottomX < rightTopX) {
                            // bottom is to the left of the top
                            ctx.arcTo(rightTopX, rightMiddleY, rightTopX - radius, rightMiddleY, radius)
                            ctx.arcTo(rightBottomX, rightMiddleY, rightBottomX, rightMiddleY + radius, radius)
                            ctx.lineTo(rightBottomX, bottomY)
                        }
                        else if (rightBottomX > rightTopX) {
                            // bottom is to the right of the top
                            ctx.arcTo(rightTopX, rightMiddleY, rightTopX + radius, rightMiddleY, radius)
                            ctx.arcTo(rightBottomX, rightMiddleY, rightBottomX, rightMiddleY + radius, radius)
                            ctx.lineTo(rightBottomX, bottomY)
                        }
                        else {
                            // straight vertical line
                            ctx.lineTo(rightBottomX, bottomY)
                        }
                        if (my.drawShadows) {
                            ctx.shadowColor   = "#222222"
                        }
                        ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                        ctx.lineWidth = lineWidth
                        ctx.stroke()
                        my.nrOfDrawCalls++
                    }
                }
                
                if (fillColorAlpha) {
                    // Draw background
                    
                    ctx.beginPath()
                    
                    // Left side (top to bottom)
                    ctx.moveTo(leftTopX, topY)
                    if (leftBottomX < leftTopX) {
                        // bottom is to the left of the top
                        ctx.arcTo(leftTopX, leftMiddleY, leftTopX - radius, leftMiddleY, radius)
                        ctx.arcTo(leftBottomX, leftMiddleY, leftBottomX, leftMiddleY + radius, radius)
                        ctx.lineTo(leftBottomX, bottomY)
                    }
                    else if (leftBottomX > leftTopX) {
                        // bottom is to the right of the top
                        ctx.arcTo(leftTopX, leftMiddleY, leftTopX + radius, leftMiddleY, radius)
                        ctx.arcTo(leftBottomX, leftMiddleY, leftBottomX, leftMiddleY + radius, radius)
                        ctx.lineTo(leftBottomX, bottomY)
                    }
                    else {
                        // straight vertical line
                        ctx.lineTo(leftBottomX, bottomY)
                    }
                    
                    // Right side (bottom to top)
                    ctx.lineTo(rightBottomX, bottomY)
                    if (rightBottomX < rightTopX) {
                        // bottom is to the left of the top
                        ctx.arcTo(rightBottomX, rightMiddleY, rightBottomX + radius, rightMiddleY, radius)
                        ctx.arcTo(rightTopX, rightMiddleY, rightTopX, rightMiddleY - radius, radius)
                        ctx.lineTo(rightTopX, topY)
                    }
                    else if (rightBottomX > rightTopX) {
                        // bottom is to the right of the top
                        ctx.arcTo(rightBottomX, rightMiddleY, rightBottomX - radius, rightMiddleY, radius)
                        ctx.arcTo(rightTopX, rightMiddleY, rightTopX, rightMiddleY - radius, radius)
                        ctx.lineTo(rightTopX, topY)
                    }
                    else {
                        // straight vertical line
                        ctx.lineTo(rightTopX, topY)
                    }
                    
                    ctx.closePath()
                
                    if (my.drawShadows) {
                        ctx.shadowColor = "transparent";
                    }
                    ctx.fillStyle = my.getCanvasRGBAColor(fillColorRGB, fillColorAlpha)
                    ctx.fill()
                    my.nrOfDrawCalls++
                }
                
                if (lineColorAlpha) {
                    // Draw left side
                    ctx.beginPath()     
                    ctx.moveTo(leftTopX, topY)
                    if (leftBottomX < leftTopX) {
                        // bottom is to the left of the top
                        ctx.arcTo(leftTopX, leftMiddleY, leftTopX - radius, leftMiddleY, radius)
                        ctx.arcTo(leftBottomX, leftMiddleY, leftBottomX, leftMiddleY + radius, radius)
                        ctx.lineTo(leftBottomX, bottomY)
                    }
                    else if (leftBottomX > leftTopX) {
                        // bottom is to the right of the top
                        ctx.arcTo(leftTopX, leftMiddleY, leftTopX + radius, leftMiddleY, radius)
                        ctx.arcTo(leftBottomX, leftMiddleY, leftBottomX, leftMiddleY + radius, radius)
                        ctx.lineTo(leftBottomX, bottomY)
                    }
                    else {
                        // straight vertical line
                        ctx.lineTo(leftBottomX, bottomY)
                    }
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                    
                    // Draw right side
                    
                    ctx.beginPath()     
                    ctx.moveTo(rightTopX, topY)
                    if (rightBottomX < rightTopX) {
                        // bottom is to the left of the top
                        ctx.arcTo(rightTopX, rightMiddleY, rightTopX - radius, rightMiddleY, radius)
                        ctx.arcTo(rightBottomX, rightMiddleY, rightBottomX, rightMiddleY + radius, radius)
                        ctx.lineTo(rightBottomX, bottomY)
                    }
                    else if (rightBottomX > rightTopX) {
                        // bottom is to the right of the top
                        ctx.arcTo(rightTopX, rightMiddleY, rightTopX + radius, rightMiddleY, radius)
                        ctx.arcTo(rightBottomX, rightMiddleY, rightBottomX, rightMiddleY + radius, radius)
                        ctx.lineTo(rightBottomX, bottomY)
                    }
                    else {
                        // straight vertical line
                        ctx.lineTo(rightBottomX, bottomY)
                    }
                    if (my.drawShadows) {
                        ctx.shadowColor = "transparent";
                    }
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
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
            
            _jsDrawLine: function (xStart, yStart, xEnd, yEnd, lineColorRGB, lineColorAlpha, lineWidth) {

                // TODO: figure out when exactly to add 0.5!
                //xStart += 0.5
                //yStart += 0.5
                //xEnd += 0.5
                //yEnd += 0.5
                
                ctx.beginPath()
                ctx.moveTo(xStart, yStart)
                ctx.lineTo(xEnd, yEnd)

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
        }
        
        return exportedFunctions
    }

    return my
}()
