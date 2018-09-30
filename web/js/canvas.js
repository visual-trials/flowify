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

var Flowify = {}

Flowify.canvas = function () {

    var my = {}

    my.canvasElement = document.getElementById("canvas")
    my.context2d = my.canvasElement.getContext("2d")

    my.fixedCanvasSize = null

    my.isIE = false

    // Check if IE
    var ua = window.navigator.userAgent;
    if (ua.indexOf('MSIE ') > 0 || ua.indexOf('Trident/') > 0 || ua.indexOf('Edge/') > 0) {
        my.isIE = true
    }

    var requestAnimFrame = function () {
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
        // TODO: set globalCompositeOperation?
        my.context2d.clearRect(0, 0, my.canvasElement.width, my.canvasElement.height)
    }
    
    my.getCanvasRGBAColor = function (colorRGB, colorAlpha) {
        
        var red = colorRGB % 256
        colorRGB = (colorRGB - red) / 256
        var green = colorRGB % 256
        colorRGB = (colorRGB - green) / 256
        var blue = colorRGB % 256
        
        return "rgba(" + red + "," + green + "," + blue + "," + colorAlpha/255 + ")"
    }
    
    my.getExportedFunctions = function () {
        
        var ctx = my.context2d
        
        var exportedFunctions = {
        
            _jsClearRect: function (x, y, width, height) {
                ctx.clearRect(x, y, width, height)
                ctx.beginPath()   // see: http://codetheory.in/why-clearrect-might-not-be-clearing-canvas-pixels/        
                ctx.closePath()
            },
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
            
            // FIXME: allow logging strings!
            _jsLogInt: function(logInt) {
                console.log(logInt)
            }
        }
        
        return exportedFunctions
    }

    return my
}()
