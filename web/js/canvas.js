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

    my.fixedCanvasSize = null

    my.isIE = false

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

    // Resizing Canvas

    my.resizeCanvas = function () {
        if (my.fixedCanvasSize == null) {
            my.resizeCanvasToWindowSize()
        }
        else {
            my.resizeCanvasToCustomSize(my.fixedCanvasSize)
        }
    }

    my.resizeCanvasToWindowSize = function () {
        if ( my.canvasElement.width != window.innerWidth || my.canvasElement.height != window.innerHeight) {
            my.canvasElement.width = window.innerWidth
            my.canvasElement.height = window.innerHeight
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
        
            _jsDrawRect: function (x, y, width, height, lineColorRGB, lineColorAlpha, fillColorRGB, fillColorAlpha, lineWidth) {
                ctx.beginPath()
                ctx.rect(x, y, width, height)

                if (fillColorAlpha) {
                    ctx.fillStyle = my.getCanvasRGBAColor(fillColorRGB, fillColorAlpha)
                    ctx.fill()
                }

                if (lineColorAlpha) {
                    ctx.strokeStyle = my.getCanvasRGBAColor(lineColorRGB, lineColorAlpha)
                    ctx.lineWidth = lineWidth
                    ctx.stroke()
                }
                
            },

            _jsDrawText: function (x, y, stringIndex, stringLength, fontHeight, fontColorRGB, fontColorAlpha) { // , baseFontIndex, baseFontLength) {
                let string = ""
                for (let i = stringIndex; i < stringIndex + stringLength; i++) {
                    string += String.fromCharCode(Flowify.main.bufferU8[i])
                }
                let baseFont = "Arial"
                /*
                let baseFont = ""
                for (let i = baseFontIndex; i < baseFontIndex + baseFontLength; i++) {
                    baseFont += String.fromCharCode(Flowify.main.bufferU8[i])
                }
                */

                // Note: fontHeight is the distance between the top of a capital and the bottom of a capital (note that a non-capital can stick out below!)
                let fontHeight2FontPx = 1.3  // TODO: is this multiplier correct?
                let fontPx = fontHeight * fontHeight2FontPx

                ctx.font = fontPx + "px " + baseFont
                let fontHeightPx = fontHeight
                ctx.fillStyle = my.getCanvasRGBAColor(fontColorRGB, fontColorAlpha)
                ctx.fillText(string, x, y + fontHeightPx)
            },

            _jsLog: function(stringIndex, stringLength) {
                let string = ""
                for (let i = stringIndex; i < stringIndex + stringLength; i++) {
                    string += String.fromCharCode(Flowify.main.bufferU8[i])
                }
                console.log(string)
            },
            
            _jsLogInt: function(logInteger) {
                console.log(logInteger)
            }
            
        }
        
        return exportedFunctions
    }

    return my
}()
