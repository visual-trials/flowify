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
                
// FIXME: we currently start with a LineWhenBackhround. But we *SHOULD* start with a Move instead!
// FIXME2: we currently have path with 0 length! We shouldn't send these to the renderer!
if (!my.alreadyLogged) {
    my.alreadyLogged = 0
}
if (my.alreadyLogged < 3) {
    console.log(leftPathParts)
    console.log(rightPathParts)
    my.alreadyLogged++
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
