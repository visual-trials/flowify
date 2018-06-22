/*

   Copyright 2017 Jeffrey Hullekes

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

ZUI.render = function () {

    var my = {}

    // my.characterWidths = {}   // FIXME: this should be removed!

    my.getCanvasRGBAColor = function (color, opacity) {
        return "rgba(" + color.r + "," + color.g + "," + color.b + "," + color.a * opacity + ")"
    }



    my.fillAndDrawRect = function (camera, x, y, width, height, fillColor, lineColor, lineWidth, opacity) {
        var context2d = camera.context2d

        var widthPx = width * camera.pixelsPerMeter
        var heightPx = height * camera.pixelsPerMeter
        var lineWidthPx = lineWidth * camera.pixelsPerMeter
        var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
        var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter - heightPx

        // Note: only drawing the portion that fits inside the camera!
        if (
            leftPx <= camera.pixelPosition.leftPx + camera.pixelSize.widthPx &&
            leftPx + widthPx >= camera.pixelPosition.leftPx &&
            topPx <= camera.pixelPosition.topPx + camera.pixelSize.heightPx &&
            topPx + heightPx >= camera.pixelPosition.topPx
        ) {

            context2d.beginPath()
            context2d.rect(leftPx, topPx, widthPx, heightPx)

            if (fillColor != null) {
                context2d.fillStyle = my.getCanvasRGBAColor(fillColor, opacity)
                context2d.fill()
            }
            if (lineColor != null) {
                context2d.strokeStyle = my.getCanvasRGBAColor(lineColor, opacity)
                context2d.lineWidth = lineWidthPx
                context2d.stroke()
            }
        }
    }

    my.fillAndDrawEllipse = function (camera, x, y, width, height, fillColor, lineColor, lineWidth, opacity) {
        var context2d = camera.context2d

        var widthPx = width * camera.pixelsPerMeter
        var heightPx = height * camera.pixelsPerMeter
        var lineWidthPx = lineWidth * camera.pixelsPerMeter
        var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
        var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter - heightPx

        function drawEllipseIE(ctx, x, y, w, h) {

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

        // Note: only drawing the portion that fits inside the camera!
        if (
            leftPx <= camera.pixelPosition.leftPx + camera.pixelSize.widthPx &&
            leftPx + widthPx >= camera.pixelPosition.leftPx &&
            topPx <= camera.pixelPosition.topPx + camera.pixelSize.heightPx &&
            topPx + heightPx >= camera.pixelPosition.topPx
        ) {

            context2d.beginPath()
            if (ZUI.canvas.isIE) {
                drawEllipseIE(context2d, leftPx, topPx, widthPx, heightPx)
            }
            else {
                context2d.ellipse(leftPx + widthPx / 2, topPx + heightPx / 2, widthPx / 2, heightPx / 2, 0, 0, 2 * Math.PI);
            }

            if (fillColor != null) {
                context2d.fillStyle = my.getCanvasRGBAColor(fillColor, opacity)
                context2d.fill()
            }
            if (lineColor != null) {
                context2d.strokeStyle = my.getCanvasRGBAColor(lineColor, opacity)
                context2d.lineWidth = lineWidthPx
                context2d.stroke()
            }

        }
    }

    my.fillAndDrawTriangleLeftBottom = function (camera, x, y, width, height, fillColor, lineColor, lineWidth, opacity) {
        var context2d = camera.context2d

        var widthPx = width * camera.pixelsPerMeter
        var heightPx = height * camera.pixelsPerMeter
        var lineWidthPx = lineWidth * camera.pixelsPerMeter
        var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
        var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter - heightPx

        if (
            leftPx <= camera.pixelPosition.leftPx + camera.pixelSize.widthPx &&
            leftPx + widthPx >= camera.pixelPosition.leftPx &&
            topPx <= camera.pixelPosition.topPx + camera.pixelSize.heightPx &&
            topPx + heightPx >= camera.pixelPosition.topPx
        ) {

            context2d.beginPath()
            context2d.moveTo(leftPx, topPx)
            context2d.lineTo(leftPx, topPx + heightPx)
            context2d.lineTo(leftPx + widthPx, topPx + heightPx)
            context2d.closePath()

            if (fillColor != null) {
                context2d.fillStyle = my.getCanvasRGBAColor(fillColor, opacity)
                context2d.fill()
            }

            if (lineColor != null) {
                context2d.strokeStyle = my.getCanvasRGBAColor(lineColor, opacity)
                context2d.lineWidth = lineWidthPx
                context2d.stroke()
            }

        }
    }

    my.fillAndDrawDiamond = function (camera, x, y, width, height, fillColor, lineColor, lineWidth, opacity) {
        var context2d = camera.context2d

        var widthPx = width * camera.pixelsPerMeter
        var heightPx = height * camera.pixelsPerMeter
        var lineWidthPx = lineWidth * camera.pixelsPerMeter
        var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
        var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter - heightPx

        context2d.beginPath()
        context2d.moveTo(leftPx + widthPx / 2, topPx)
        context2d.lineTo(leftPx, topPx + heightPx / 2)
        context2d.lineTo(leftPx + widthPx / 2, topPx + heightPx)
        context2d.lineTo(leftPx + widthPx, topPx + heightPx / 2)
        context2d.closePath()

        if (
            leftPx <= camera.pixelPosition.leftPx + camera.pixelSize.widthPx &&
            leftPx + widthPx >= camera.pixelPosition.leftPx &&
            topPx <= camera.pixelPosition.topPx + camera.pixelSize.heightPx &&
            topPx + heightPx >= camera.pixelPosition.topPx
        ) {

            if (fillColor != null) {
                context2d.fillStyle = my.getCanvasRGBAColor(fillColor, opacity)
                context2d.fill()
            }

            if (lineColor != null) {
                context2d.strokeStyle = my.getCanvasRGBAColor(lineColor, opacity)
                context2d.lineWidth = lineWidthPx
                context2d.stroke()
            }
        }
    }

    my.fillAndDrawPentagonFlatBottom = function (camera, x, y, width, height, fillColor, lineColor, lineWidth, opacity) {
        var context2d = camera.context2d

        var widthPx = width * camera.pixelsPerMeter
        var heightPx = height * camera.pixelsPerMeter
        var lineWidthPx = lineWidth * camera.pixelsPerMeter
        var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
        var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter - heightPx

        if (
            leftPx <= camera.pixelPosition.leftPx + camera.pixelSize.widthPx &&
            leftPx + widthPx >= camera.pixelPosition.leftPx &&
            topPx <= camera.pixelPosition.topPx + camera.pixelSize.heightPx &&
            topPx + heightPx >= camera.pixelPosition.topPx
        ) {

            // TODO: do this more accurately

            context2d.beginPath()
            context2d.moveTo(leftPx + widthPx / 2, topPx)
            context2d.lineTo(leftPx + widthPx * (0.2 / 8), topPx + heightPx * (3.5 / 8))
            context2d.lineTo(leftPx + widthPx * (1 / 4), topPx + heightPx * (7.5 / 8))
            context2d.lineTo(leftPx + widthPx * (3 / 4), topPx + heightPx * (7.5 / 8))
            context2d.lineTo(leftPx + widthPx * (7.8 / 8), topPx + heightPx * (3.5 / 8))
            context2d.closePath()

            if (fillColor != null) {
                context2d.fillStyle = my.getCanvasRGBAColor(fillColor, opacity)
                context2d.fill()
            }

            if (lineColor != null) {
                context2d.strokeStyle = my.getCanvasRGBAColor(lineColor, opacity)
                context2d.lineWidth = lineWidthPx
                context2d.stroke()
            }
        }
    }

    my.fillAndDrawCircle = function (camera, x, y, width, height, fillColor, lineColor, lineWidth, opacity) {
        var context2d = camera.context2d

        var widthPx = width * camera.pixelsPerMeter
        var heightPx = height * camera.pixelsPerMeter
        var lineWidthPx = lineWidth * camera.pixelsPerMeter
        var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
        var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter - heightPx

        if (
            leftPx <= camera.pixelPosition.leftPx + camera.pixelSize.widthPx &&
            leftPx + widthPx >= camera.pixelPosition.leftPx &&
            topPx <= camera.pixelPosition.topPx + camera.pixelSize.heightPx &&
            topPx + heightPx >= camera.pixelPosition.topPx
        ) {

            // TODO: do this more accurately

            // TODO: heightPx and widthPx are presumed to be the same!
            context2d.beginPath()
            context2d.arc(leftPx + widthPx / 2, topPx + heightPx / 2, heightPx / 2, 0, 2 * Math.PI)

            if (fillColor != null) {
                context2d.fillStyle = my.getCanvasRGBAColor(fillColor, opacity)
                context2d.fill()
            }

            if (lineColor != null) {
                context2d.strokeStyle = my.getCanvasRGBAColor(lineColor, opacity)
                context2d.lineWidth = lineWidthPx
                context2d.stroke()
            }
        }

    }

    my.drawBezierCurve = function (camera, fromX, fromY, fromBendingX, fromBendingY, toBendingX, toBendingY, toX, toY, lineColor, lineWidth, opacity) {
        var context2d = camera.context2d

        // FIXME: only draw the portion that fits inside the camera!
        // FIXME: only draw the portion that fits inside the camera!
        // FIXME: only draw the portion that fits inside the camera!
        // FIXME: only draw the portion that fits inside the camera!

        var cameraCenterPixelPosition = {
            leftPx: camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2,
            topPx: camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2
        }

        var fromLeftPx = cameraCenterPixelPosition.leftPx + (fromX - camera.centerPosition.x) * camera.pixelsPerMeter
        var fromTopPx = cameraCenterPixelPosition.topPx - (fromY - camera.centerPosition.y) * camera.pixelsPerMeter

        var fromBendingLeftPx = cameraCenterPixelPosition.leftPx + (fromBendingX - camera.centerPosition.x) * camera.pixelsPerMeter
        var fromBendingTopPx = cameraCenterPixelPosition.topPx - (fromBendingY - camera.centerPosition.y) * camera.pixelsPerMeter

        var toBendingLeftPx = cameraCenterPixelPosition.leftPx + (toBendingX - camera.centerPosition.x) * camera.pixelsPerMeter
        var toBendingTopPx = cameraCenterPixelPosition.topPx - (toBendingY - camera.centerPosition.y) * camera.pixelsPerMeter

        var toLeftPx = cameraCenterPixelPosition.leftPx + (toX - camera.centerPosition.x) * camera.pixelsPerMeter
        var toTopPx = cameraCenterPixelPosition.topPx - (toY - camera.centerPosition.y) * camera.pixelsPerMeter

        var lineWidthPx = lineWidth * camera.pixelsPerMeter

        if (lineColor != null) {
            context2d.lineWidth = lineWidthPx
            context2d.strokeStyle = my.getCanvasRGBAColor(lineColor, opacity)

            context2d.beginPath()
            context2d.moveTo(fromLeftPx, fromTopPx)
            context2d.bezierCurveTo(fromBendingLeftPx, fromBendingTopPx, toBendingLeftPx, toBendingTopPx, toLeftPx, toTopPx)
            context2d.stroke()
        }
    }

    my.drawArrowHead = function (camera, arrowPointX, arrowPointY, arrowPointLeftX, arrowPointLeftY, arrowPointRightX, arrowPointRightY, fillColor, opacity) {
        var context2d = camera.context2d

        // TODO: only draw the portion that fits inside the camera!

        var cameraCenterPixelPosition = {
            leftPx: camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2,
            topPx: camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2
        }

        var arrowPointLeftPx = cameraCenterPixelPosition.leftPx + (arrowPointX - camera.centerPosition.x) * camera.pixelsPerMeter
        var arrowPointTopPx = cameraCenterPixelPosition.topPx - (arrowPointY - camera.centerPosition.y) * camera.pixelsPerMeter

        var arrowPointLeftLeftPx = cameraCenterPixelPosition.leftPx + (arrowPointLeftX - camera.centerPosition.x) * camera.pixelsPerMeter
        var arrowPointLeftTopPx = cameraCenterPixelPosition.topPx - (arrowPointLeftY - camera.centerPosition.y) * camera.pixelsPerMeter

        var arrowPointRightLeftPx = cameraCenterPixelPosition.leftPx + (arrowPointRightX - camera.centerPosition.x) * camera.pixelsPerMeter
        var arrowPointRightTopPx = cameraCenterPixelPosition.topPx - (arrowPointRightY - camera.centerPosition.y) * camera.pixelsPerMeter

        if (fillColor != null) {
            context2d.fillStyle = my.getCanvasRGBAColor(fillColor, opacity)

            context2d.beginPath()
            context2d.moveTo(arrowPointLeftPx, arrowPointTopPx)
            context2d.lineTo(arrowPointLeftLeftPx, arrowPointLeftTopPx)
            context2d.lineTo(arrowPointRightLeftPx, arrowPointRightTopPx)
            context2d.closePath()
            context2d.fill()
        }
    }

    /*
    my.measureTextWidth = function (camera, text, fontHeight, baseFont) {
        // FIXME: we can't use this here right now! (since it might be pipelined) var context2d = camera.context2d
        var context2d = ZUI.canvas.context2d

        // TODO: we should be able to measureText without a canvas!

        // TODO: is this multiplier correct?
        // Note: fontHeight is the distance between the top of a capital and the bottom of a capital (note that a non-capital can stick out below!)
        var fontHeight2FontPx = 1.3
        var fontPx = Math.round(fontHeight * fontHeight2FontPx * camera.pixelsPerMeter)

        context2d.font = fontPx + "px " + baseFont

        var textWidth = 0
        for (var charIndex = 0; charIndex < text.length; charIndex++) {
            var character = text.charAt(charIndex);

            if (!my.characterWidths.hasOwnProperty(fontPx)) {
                my.characterWidths[fontPx] = {}
            }

            var characterWidth = null
            if (fontPx <= 40) {  //  caching...
                if (!my.characterWidths[fontPx].hasOwnProperty(character)) {
                    my.characterWidths[fontPx][character] = context2d.measureText(character).width
                }

                characterWidth = my.characterWidths[fontPx][character]
            }
            else {
                characterWidth = context2d.measureText(character).width
            }

            textWidth += characterWidth
        }

        return textWidth
    }
    */

    my.drawText = function (camera, text, x, y, fontHeight, fontColor, baseFont, opacity) {
        var context2d = camera.context2d

        // TODO: only draw the portion that fits inside the camera!

        var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
        var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter

        // TODO: is this multiplier correct?
        // Note: fontHeight is the distance between the top of a capital and the bottom of a capital (note that a non-capital can stick out below!)
        var fontHeight2FontPx = 1.3
        var fontPx = fontHeight * fontHeight2FontPx * camera.pixelsPerMeter
        context2d.font = fontPx + "px " + baseFont

        var fontHeightPx = fontHeight * camera.pixelsPerMeter

        context2d.fillStyle = my.getCanvasRGBAColor(fontColor, opacity)

        // Not drawing text smaller than 2 px in height
        if (fontHeightPx >= 2) {
            context2d.fillText(text, leftPx, topPx + fontHeightPx)
            // TODO: restore this! context2d.fillText(text, leftPx, topPx)
        }

    }

    /*
    my.drawTextInRect = function (camera, text, x, y, width, height, centerHorizontally, centerVertically, fontHeight, fontColor, baseFont, opacity, cutOffTextWhenNoMoreRoom, wrapTextWhenNoMoreRoom) {

        // Note: DEBUG my.fillAndDrawRect(camera, x, y, width, height, null, {r: 200, g:0, b:0, a:1.0}, 1)

        // TODO: is there a better way so ensure the text is converted to text?
        text = text + ''

        // TODO: we are calculating pixels here, but using meters again in drawText() we subsequently call...

        var widthPx = width * camera.pixelsPerMeter
        var heightPx = height * camera.pixelsPerMeter
        var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
        var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter - heightPx

        // Note: only drawing the portion that fits inside the camera!
        if (
            leftPx <= camera.pixelPosition.leftPx + camera.pixelSize.widthPx &&
            leftPx + widthPx >= camera.pixelPosition.leftPx &&
            topPx <= camera.pixelPosition.topPx + camera.pixelSize.heightPx &&
            topPx + heightPx >= camera.pixelPosition.topPx
        ) {

            // TODO: implement centerVertically here (not just use the size of the font to fit the whole height to make it centered vertically)

            // FIXME: check if the fontHeight of the is even smaller or equal to the available height, if not, dont draw any text

            var drawText = true

            var dotDotDot = '...'
            var dotDotDotWidth = my.measureTextWidth(camera, dotDotDot, fontHeight, baseFont)

            var textLine = ''
            var textLineWidth = 0
            var isFirstLine = true
            var isLastLine = false

            var textLineUntilLatestWhiteSpace = ''
            var textLineUntilLatestWhiteSpaceWidth = 0

            var textY = y + height  // You start the text at the top of the rectangle
            var textX = x

            var lineHeight = fontHeight + fontHeight * 0.5 // FIXME: HACKED multiplier

            if (textY - lineHeight <= y) {
                isLastLine = true
            }

            for (var charIndex = 0; charIndex < text.length; charIndex++) {
                var character = text.charAt(charIndex)

                // FIXME: this might be slow (the function measureTextWidth expect a string of characters, not one character)
                var characterWidth = my.measureTextWidth(camera, character, fontHeight, baseFont)

                // FIXME: do we still want to do this? (see the '...'-code below)

                // If this is the first and last line (only one line available) and the width of the text is going to be more than half of the available width, we dont show the text at all
                // if (isLastLine && isFirstLine && ((textLineWidth + characterWidth) / 2) > width) {
                //     drawText = false
                //     break
                // }

                // For word wrapping we need to remember the part of the line until the latest whitespace (or newline)
                if (character === ' ' || character === "\n") {
                    textLineUntilLatestWhiteSpace = textLine
                    textLineUntilLatestWhiteSpaceWidth = textLineWidth
                }

                if (wrapTextWhenNoMoreRoom && textLineWidth + characterWidth > width || character == "\n") {

                    if (isLastLine) {
                        break
                    }

                    if (textLineUntilLatestWhiteSpace === '') {
                        // We are about to draw the line until the latest whitespace, but apparently we haven't found a whitespace yet. So we force the break into the lline
                        textLineUntilLatestWhiteSpace = textLine
                        textLineUntilLatestWhiteSpaceWidth = textLineWidth
                    }

                    if (centerHorizontally) {
                        my.drawText(camera, textLineUntilLatestWhiteSpace, textX + (width / 2) - (textLineWidth / 2), textY, fontHeight, fontColor, baseFont, opacity)
                    }
                    else {
                        my.drawText(camera, textLineUntilLatestWhiteSpace, textX, textY, fontHeight, fontColor, baseFont, opacity)
                    }

                    textY -= lineHeight

                    // FIXME: is the +1 really correct here? Or at least what it represents correcrly commented?
                    textLine = textLine.substr(textLineUntilLatestWhiteSpace.length + 1) // take the left-over string, excluding the whitespace itself (hence the +1)
                    textLineWidth = my.measureTextWidth(camera, textLine, fontHeight, baseFont)

                    textLineUntilLatestWhiteSpace = ''
                    textLineUntilLatestWhiteSpaceWidth = 0

                    isFirstLine = false

                    if (textY - lineHeight <= y) {
                        isLastLine = true
                    }
                }

                // FIXME: we shouldnt add a whitespace if its at the beginning of a wrapped line, but we should draw it if its NOT a wrapped line
                if (character != "\n") {
                    textLine += character
                    textLineWidth += characterWidth
                }

            }

            if (drawText) {
                // Draw the last line of the text

                // FIXME: how to deal with cutOffTextWhenNoMoreRoom when you have multiple lines?!

                if (cutOffTextWhenNoMoreRoom && textLineWidth + characterWidth > width) {
                    // If the last line did't fit, we remove some characters and add '...' (only if there are 2 characters left after the removal)
                    // FIXME: don't use nrOfCharactersToRemove here as a hacked number, instead remember the last textLineWidth (and corresponding textLine) where textLineWidth + dotDotDotWidth <= width was still the case
                    var nrOfCharactersToRemove = 2
                    var minimumAmountOfCharactersToDraw = 2
                    if (textLine.length - nrOfCharactersToRemove >= minimumAmountOfCharactersToDraw) {
                        textLine = textLine.substr(0, textLine.length - nrOfCharactersToRemove)  // Note: using a negative number for the second parameter won't work for IE8

                        textLine += dotDotDot
                    }
                    else {
                        // Not enough characters to draw
                        drawText = false
                    }
                }

                if (drawText) {
                    var drawTextX = textX
                    var drawTextY = textY

                    if (centerHorizontally) {
                        drawTextX = textX + (width / 2) - (textLineWidth / 2)
                    }
                    if (centerVertically) {
                        drawTextY = textY - (height / 2) + (fontHeight / 2)
                    }

                    my.drawText(camera, textLine, drawTextX, drawTextY, fontHeight, fontColor, baseFont, opacity)
                }

            }

        }

    }
    */

    my.drawTextPx = function (camera, textToWrite, textColor, leftTextPx, topTextPx, maxTextWidthPx, lineHeightPx) {
        var context2d = camera.context2d

        // TODO: wrap text using maxTextWidthPx and lineHeightPx

        context2d.fillStyle = textColor

        // TODO: hardcoded baseFont!
        // var baseFont = '"Helvetica Neue",Helvetica,Arial,sans-serif'
        // var baseFont = 'Verdana,sans-serif'
        var baseFont = 'Arial'

        // TODO: is this multiplier correct?
        var fontPx = lineHeightPx * 0.7
        context2d.font = fontPx + "px " + baseFont

        textToWrite += '' // forcing it to a string

        var textLineUntilLatestWhiteSpace = ''
        var textLineUntilLatestWhiteSpaceWidth = 0

        var textLine = ''
        var textLineWidth = 0
        for (var charIndex = 0; charIndex < textToWrite.length; charIndex++) {
            var character = textToWrite.charAt(charIndex);

            /*

             caching...

             if (!characterWidths.hasOwnProperty(character)) {
             characterWidths[character] = context2d.measureText(character).width
             }


             var characterWidth = characterWidths[character]

             */

            var characterWidth = context2d.measureText(character).width

            // For word wrapping we need to remember the part of the line until the latest whitespace (or newline)
            if (character === ' ' || character === "\n") {
                textLineUntilLatestWhiteSpace = textLine
                textLineUntilLatestWhiteSpaceWidth = textLineWidth
            }

            // TODO: what if a single character doesn't fit in maxTextWidthPx?

            if (textLineWidth + characterWidth > maxTextWidthPx || character == "\n") {

                if (textLineUntilLatestWhiteSpace === '') {
                    // We are about to draw the line until the latest whitespace, but apparently we haven't found a whitespace yet. So we force the break into the lline
                    textLineUntilLatestWhiteSpace = textLine
                    textLineUntilLatestWhiteSpaceWidth = textLineWidth
                }

                context2d.fillText(textLineUntilLatestWhiteSpace, leftTextPx, topTextPx)
                topTextPx += lineHeightPx

                // FIXME: is the +1 really correct here? Or at least what it represents correcrly commented?
                textLine = textLine.substr(textLineUntilLatestWhiteSpace.length + 1) // take the left-over string, excluding the whitespace itself (hence the +1)
                textLineWidth = context2d.measureText(textLine).width

                textLineUntilLatestWhiteSpace = ''
                textLineUntilLatestWhiteSpaceWidth = 0

            }

            // FIXME: we shouldnt add a whitespace if its at the beginning of a wrapped line, but we should draw it if its NOT a wrapped line
            if (character != "\n") {
                textLine += character
                textLineWidth += characterWidth
            }
        }
        context2d.fillText(textLine, leftTextPx, topTextPx)
        topTextPx += lineHeightPx

        return topTextPx
    }

    my.drawIcon = function (camera, iconShape, x, y, iconWidth, iconHeight, iconColor, iconBorderColor, borderWidth, opacity) {

        if (iconShape == 'square') {
            my.fillAndDrawRect(camera, x, y, iconWidth, iconHeight, iconColor, iconBorderColor, borderWidth, opacity)
        }
        else if (iconShape == 'triangle_left_bottom') {
            my.fillAndDrawTriangleLeftBottom(camera, x, y, iconWidth, iconHeight, iconColor, iconBorderColor, borderWidth, opacity)
        }
        else if (iconShape == 'diamond') {
            my.fillAndDrawDiamond(camera, x, y, iconWidth, iconHeight, iconColor, iconBorderColor, borderWidth, opacity)
        }
        else if (iconShape == 'pentagon_flat_bottom') {
            my.fillAndDrawPentagonFlatBottom(camera, x, y, iconWidth, iconHeight, iconColor, iconBorderColor, borderWidth, opacity)
        }
        else if (iconShape == 'circle') {
            my.fillAndDrawCircle(camera, x, y, iconWidth, iconHeight, iconColor, iconBorderColor, borderWidth, opacity)
        }
        else {
            console.log('Unknown shape')
        }

    }

    my.drawBarGraphPx = function (camera, bars, nrOfBars, lastBarIndex, maxValue) {
        var context2d = camera.context2d

        var leftPx = camera.pixelPosition.leftPx
        var topPx = camera.pixelPosition.topPx
        var widthPx = camera.pixelSize.widthPx
        var heightPx = camera.pixelSize.heightPx

        context2d.clearRect(leftPx, topPx, widthPx, heightPx)

        context2d.fillStyle = "rgba(250, 250, 250, 1)"
        context2d.fillRect(leftPx, topPx, widthPx, heightPx)

        context2d.beginPath()
        context2d.lineWidth = "2"
        context2d.strokeStyle = "rgba(150, 150, 150, 0.3)"
        context2d.rect(leftPx, topPx, widthPx, heightPx)
        context2d.stroke()

        /*
        var nrOfBars = 60
        var currentDate = new Date()
        var minuteToBarOffset = currentDate.getMinutes() + 1

        var maxValue = 1;
        for (var barNr = 0; barNr < nrOfBars; barNr++) {
            var minuteToDraw = (barNr + minuteToBarOffset) % 60
            if (bars[minuteToDraw] > maxValue) {
                maxValue = bars[minuteToDraw]
            }
        }
        */

        var barMarginRightPx = 3
        var graphMarginBottomPx = 20
        var graphMarginTopPx = 20
        var graphMarginLeftPx = 20
        var graphMarginRightPx = 20

        var heightPxPerValue = (heightPx - graphMarginTopPx - graphMarginBottomPx) / maxValue
/*
        for (var valueNr = 0; valueNr <= maxValue; valueNr++) {
            context2d.fillStyle = "rgba(255, 50, 50, 0.3)"
            // TODO: this should be a line!
            context2d.fillRect(leftPx + widthPx - graphMarginRightPx + (graphMarginRightPx / 4), topPx + heightPx - graphMarginBottomPx - (valueNr * heightPxPerValue), graphMarginRightPx / 3, 1)
        }
*/
        var barWidthPx = ((widthPx - graphMarginLeftPx - graphMarginRightPx) / nrOfBars) - barMarginRightPx

        for (var barNr = 0; barNr < nrOfBars; barNr++) {
            // var minuteToDraw = (barNr + minuteToBarOffset) % 60

            var barIndex = (barNr + lastBarIndex + 1) % nrOfBars

            var barLeftPx = leftPx + graphMarginLeftPx + barNr * (barWidthPx + barMarginRightPx)

            var barHeightPx = bars[barIndex] * heightPxPerValue
            var barTopPx = topPx + heightPx - graphMarginBottomPx - barHeightPx

            context2d.fillStyle = "rgba(50, 50, 255, 0.3)"
            context2d.fillRect(barLeftPx, barTopPx, barWidthPx, barHeightPx)

        }

    }

    my.drawContainerInfoGUIPx = function (camera, currentSliceContainer) {
        var context2d = camera.context2d

        var leftPx = camera.pixelPosition.leftPx
        var topPx = camera.pixelPosition.topPx
        var widthPx = camera.pixelSize.widthPx
        var heightPx = camera.pixelSize.heightPx

        context2d.clearRect(leftPx, topPx, widthPx, heightPx)

        context2d.fillStyle = "rgba(255, 255, 255, 1)"
        context2d.fillRect(leftPx, topPx, widthPx, heightPx)

/*
        context2d.beginPath()
        context2d.lineWidth = "1"
        context2d.strokeStyle = "rgba(150, 150, 150, 0.3)"
        context2d.rect(leftPx, topPx, widthPx, heightPx)
        context2d.stroke()
        */


        /*
        context2d.fillStyle = "rgba(200, 200, 200, 1)"
        context2d.beginPath();
        context2d.lineWidth = "3"
        context2d.lineJoin = "round";
        context2d.moveTo(leftPx + 40, topPx + heightPx - 80);
        context2d.lineTo(leftPx + 40 + 100, topPx + heightPx - 80);
        context2d.lineTo(leftPx + 40 + 100, topPx + heightPx - 80 + 40);
        context2d.lineTo(leftPx + 40, topPx + heightPx - 80 + 40);
        context2d.closePath();
        // context2d.stroke();
        context2d.fill();
        // context2d.fillRect(, , , )
        */

        if (currentSliceContainer != null) {
            // var textToWrite = JSON.stringify(currentSliceContainer)
            var horizontalMarginTextPx = 30
            var verticalMarginTextPx = 40
            var lineHeightPx = 30
            var horizontalIndentPx = 0
            var verticalWhiteSpacePx = 20
            var leftTextPx = leftPx + horizontalMarginTextPx
            var maxTextWidthPx = widthPx - horizontalMarginTextPx - horizontalMarginTextPx
            var topTextPx = topPx + verticalMarginTextPx

            var textColorPlain = "rgba(100, 100, 100, 0.40)"
            var textColorBold = "rgba(100, 100, 100, 0.70)"

            var containerData = currentSliceContainer.worldContainer.containerData
            var containerProperties = currentSliceContainer.worldContainer.containerProperties
            var dataItemGroups = containerProperties.dataItemGroups

            var dataItemsToShowOnSideGUI = []
            if (dataItemGroups.hasOwnProperty('basicInfo')) {
                dataItemsToShowOnSideGUI = dataItemGroups.basicInfo

                for (var loopIndex = 0; loopIndex < dataItemsToShowOnSideGUI.length; loopIndex++) {
                    var dataItemToShowOnSideGUI = dataItemsToShowOnSideGUI[loopIndex]

                    var displayName = null
                    var value = null

                    if (dataItemToShowOnSideGUI.hasOwnProperty('displayName')) {
                        if (dataItemToShowOnSideGUI.hasOwnProperty('containerDataKey')) {
                            var containerDataKey = dataItemToShowOnSideGUI.containerDataKey
                            displayName = dataItemToShowOnSideGUI.displayName

                            if (containerData.hasOwnProperty(containerDataKey)) {
                                value = containerData[containerDataKey]
                            }
                        }
                        else if (dataItemToShowOnSideGUI.hasOwnProperty('containerKey')) {
                            var containerKey = dataItemToShowOnSideGUI.containerKey
                            displayName = dataItemToShowOnSideGUI.displayName

                            if (currentSliceContainer.worldContainer.hasOwnProperty(containerKey)) {
                                value = currentSliceContainer.worldContainer[containerKey]
                            }
                        }
                        else if (dataItemToShowOnSideGUI.hasOwnProperty('containerPropertyKey')) {
                            var containerPropertyKey = dataItemToShowOnSideGUI.containerPropertyKey
                            displayName = dataItemToShowOnSideGUI.displayName

                            if (containerProperties.hasOwnProperty(containerPropertyKey)) {
                                value = containerProperties[containerPropertyKey]
                            }
                        }
                        else {
                            ZUI.log('No containerDataKey or containerKey given')
                        }

                        topTextPx = my.drawTextPx(camera, displayName, textColorBold, leftTextPx, topTextPx, maxTextWidthPx, lineHeightPx)
                        topTextPx = my.drawTextPx(camera, value, textColorPlain, leftTextPx + horizontalIndentPx, topTextPx, maxTextWidthPx - horizontalIndentPx, lineHeightPx - 4)
                        topTextPx += verticalWhiteSpacePx
                    }
                    else {
                        ZUI.log('No displayName given')
                    }
                }
            }
            else {
                // FIXME: add container properties to callers and remove below

                topTextPx = my.drawTextPx(camera, 'type', textColorBold, leftTextPx, topTextPx, maxTextWidthPx, lineHeightPx)
                topTextPx = my.drawTextPx(camera, currentSliceContainer.worldContainer.type, textColorPlain, leftTextPx + horizontalIndentPx, topTextPx, maxTextWidthPx - horizontalIndentPx, lineHeightPx)
                topTextPx += verticalWhiteSpacePx

                topTextPx = my.drawTextPx(camera, 'identifier', textColorBold, leftTextPx, topTextPx, maxTextWidthPx, lineHeightPx)
                topTextPx = my.drawTextPx(camera, currentSliceContainer.worldContainer.identifier, textColorPlain, leftTextPx + horizontalIndentPx, topTextPx, maxTextWidthPx - horizontalIndentPx, lineHeightPx)
                topTextPx += verticalWhiteSpacePx

                if (containerData.label) {
                    topTextPx = my.drawTextPx(camera, 'label', textColorBold, leftTextPx, topTextPx, maxTextWidthPx, lineHeightPx)
                    topTextPx = my.drawTextPx(camera, containerData.label, textColorPlain, leftTextPx + horizontalIndentPx, topTextPx, maxTextWidthPx - horizontalIndentPx, lineHeightPx)
                    topTextPx += verticalWhiteSpacePx
                }

                if (currentSliceContainer.containerType == 'process') {
                    if (containerData.executable != null) {
                        topTextPx = my.drawTextPx(camera, 'executable', textColorBold, leftTextPx, topTextPx, maxTextWidthPx, lineHeightPx)
                        topTextPx = my.drawTextPx(camera, containerData.executable, textColorPlain, leftTextPx + horizontalIndentPx, topTextPx, maxTextWidthPx - horizontalIndentPx, lineHeightPx)
                        topTextPx += verticalWhiteSpacePx
                    }

                    if (containerData.arguments != null) {
                        topTextPx = my.drawTextPx(camera, 'arguments', textColorBold, leftTextPx, topTextPx, maxTextWidthPx, lineHeightPx)
                        topTextPx = my.drawTextPx(camera, containerData.arguments, textColorPlain, leftTextPx + horizontalIndentPx, topTextPx, maxTextWidthPx - horizontalIndentPx, lineHeightPx)
                        topTextPx += verticalWhiteSpacePx
                    }
                }

                if (currentSliceContainer.containerType == 'storage') {
                    if (containerData.location != null) {
                        topTextPx = my.drawTextPx(camera, 'location', textColorBold, leftTextPx, topTextPx, maxTextWidthPx, lineHeightPx)
                        topTextPx = my.drawTextPx(camera, containerData.location, textColorPlain, leftTextPx + horizontalIndentPx, topTextPx, maxTextWidthPx - horizontalIndentPx, lineHeightPx)
                        topTextPx += verticalWhiteSpacePx
                    }

                    if (currentSliceContainer.worldContainer.containerData.nrOfFiles != null) {
                        topTextPx = my.drawTextPx(camera, 'nrOfFiles', textColorBold, leftTextPx, topTextPx, maxTextWidthPx, lineHeightPx)
                        topTextPx = my.drawTextPx(camera, containerData.nrOfFiles.toString(), textColorPlain, leftTextPx + horizontalIndentPx, topTextPx, maxTextWidthPx - horizontalIndentPx, lineHeightPx)
                        topTextPx += verticalWhiteSpacePx
                    }
                }

            }

        }

    }

    my.drawContainerNew = function (camera, sliceContainer, drawingAfterConnections) {

        var containerProperties = sliceContainer.worldContainer.containerProperties

        var drawAfterConnections = containerProperties.drawAfterConnections

        if (drawingAfterConnections !== drawAfterConnections) {
            return
        }

        var opacity = sliceContainer.opacity

        var showContainerBody = containerProperties.showContainerBody
        var defaultContainerColor = containerProperties.containerColor
        var defaultContainerBorderColor = containerProperties.containerBorderColor

        var isAlerting = containerProperties.isAlerting
        var alertStartTime = containerProperties.alertStartTime
        var alertDuration = containerProperties.alertDuration
        var alertProgress = 0;

        if (isAlerting) {
            var proceededTime = ZUI.main.currentTime - alertStartTime
            if (proceededTime > alertDuration) {
                containerProperties.isAlerting = false
                isAlerting = false
            }
            else {
                alertProgress = proceededTime / alertDuration
            }
        }

        var containerIsSelected = false
        if (camera.selectedSliceContainer != null && camera.selectedSliceContainer.worldContainer.identifier === sliceContainer.worldContainer.identifier) {
            containerIsSelected = true
        }

        var containerIsBeingHovered = false
        if (camera.hoveringSliceContainer != null && camera.hoveringSliceContainer.worldContainer.identifier === sliceContainer.worldContainer.identifier) {
            containerIsBeingHovered = true
        }

        var width = sliceContainer.newLayout.absoluteSize.width
        var height = sliceContainer.newLayout.absoluteSize.height

        var absoluteLeftBottomPosition = ZUI.world.findPositionInShape(sliceContainer.newLayout.absolutePosition, sliceContainer.newLayout.isPositionOf, containerProperties.shape, sliceContainer.newLayout.absoluteSize, 'left-bottom')

        var x = absoluteLeftBottomPosition.x
        var y = absoluteLeftBottomPosition.y

        // FIXME: much of below is assuming a rectangle container! Should be based on its shape!

        if (showContainerBody) {
            //
            // Draw the basic container
            //

            var borderWidth = 2  // FIXME: hardcoded!

            var containerColor = defaultContainerColor
            var containerBorderColor = defaultContainerBorderColor
            var selectedContainerBorderColor = defaultContainerBorderColor

            var containerColorMultiplier = 1.0
            var containerBorderColorMultiplier = 1.0
            if (containerIsSelected) {
                containerColorMultiplier = 0.7
                containerBorderColorMultiplier = 0.7

                containerColor = {
                    r: 255 - Math.round((255 - containerColor.r) * containerColorMultiplier),
                    g: 255 - Math.round((255 - containerColor.g) * containerColorMultiplier),
                    b: 255 - Math.round((255 - containerColor.b) * containerColorMultiplier),
                    a: containerColor.a
                }

                selectedContainerBorderColor = {
                    r: 255 - Math.round((255 - containerBorderColor.r) * containerBorderColorMultiplier),
                    g: 255 - Math.round((255 - containerBorderColor.g) * containerBorderColorMultiplier),
                    b: 255 - Math.round((255 - containerBorderColor.b) * containerBorderColorMultiplier),
                    a: containerBorderColor.a
                }
            }

            if (containerProperties.shape == "ellipse") {
                my.fillAndDrawEllipse(
                    camera,
                    x,
                    y,
                    width,
                    height,
                    containerColor,
                    containerBorderColor,
                    borderWidth,
                    opacity
                )

                if (isAlerting) {
                    var distanceFromContainer = 1000 * alertProgress
                    var alertColor = {
                        r: selectedContainerBorderColor.r,
                        g: selectedContainerBorderColor.g,
                        b: selectedContainerBorderColor.b,
                        a: 0.7 * (1 - alertProgress)
                    }
                    my.fillAndDrawEllipse(
                        camera,
                        x - borderWidth - distanceFromContainer / 2,
                        y - borderWidth - distanceFromContainer / 2,
                        width + borderWidth * 2 + distanceFromContainer,
                        height + borderWidth * 2 + distanceFromContainer,
                        null,
                        alertColor,
                        borderWidth * 5 + alertProgress * 5,
                        opacity
                    )
                }

                if (containerIsSelected) {
                    my.fillAndDrawEllipse(
                        camera,
                        x - borderWidth,
                        y - borderWidth,
                        width + borderWidth * 2,
                        height + borderWidth * 2,
                        null,
                        selectedContainerBorderColor,
                        borderWidth * 1,
                        opacity
                    )
                    my.fillAndDrawEllipse(
                        camera,
                        x - borderWidth * 2,
                        y - borderWidth * 2,
                        width + borderWidth * 4,
                        height + borderWidth * 4,
                        null,
                        containerBorderColor,
                        borderWidth * 1,
                        opacity * 0.3
                    )
                }
                else if (containerIsBeingHovered) {
                    my.fillAndDrawEllipse(
                        camera,
                        x - borderWidth,
                        y - borderWidth,
                        width + borderWidth * 2,
                        height + borderWidth * 2,
                        null,
                        containerBorderColor,
                        borderWidth * 2,
                        opacity * 0.5
                    )
                }
            }
            else {
                my.fillAndDrawRect(
                    camera,
                    x,
                    y,
                    width,
                    height,
                    containerColor,
                    containerBorderColor,
                    borderWidth,
                    opacity
                )

                if (isAlerting) {
                    var distanceFromContainer = 1000 * alertProgress
                    var alertColor = {
                        r: selectedContainerBorderColor.r,
                        g: selectedContainerBorderColor.g,
                        b: selectedContainerBorderColor.b,
                        a: 0.7 * (1 - alertProgress)
                    }
                    my.fillAndDrawEllipse(
                        camera,
                        x - borderWidth - distanceFromContainer / 2,
                        y - borderWidth - distanceFromContainer / 2,
                        width + borderWidth * 2 + distanceFromContainer,
                        height + borderWidth * 2 + distanceFromContainer,
                        null,
                        alertColor,
                        borderWidth * 5 + alertProgress * 5,
                        opacity
                    )
                }

                if (containerIsSelected) {
                    my.fillAndDrawRect(
                        camera,
                        x - borderWidth,
                        y - borderWidth,
                        width + borderWidth * 2,
                        height + borderWidth * 2,
                        null,
                        selectedContainerBorderColor,
                        borderWidth * 1,
                        opacity
                    )
                    my.fillAndDrawRect(
                        camera,
                        x - borderWidth * 2,
                        y - borderWidth * 2,
                        width + borderWidth * 4,
                        height + borderWidth * 4,
                        null,
                        containerBorderColor,
                        borderWidth * 1,
                        opacity * 0.3
                    )
                }
                else if (containerIsBeingHovered) {
                    my.fillAndDrawRect(
                        camera,
                        x - borderWidth,
                        y - borderWidth,
                        width + borderWidth * 2,
                        height + borderWidth * 2,
                        null,
                        containerBorderColor,
                        borderWidth * 2,
                        opacity * 0.5
                    )
                }
            }
        }

        // TODO: if containerBody is not drawn, should we show it's text?

        if (containerProperties.showContainerText) {
            //
            // Draw container text lines
            //

            var textLines = sliceContainer.textLines

            for (var loopIndex = 0; loopIndex < textLines.length; loopIndex++) {
                var textLine = textLines[loopIndex]

                // FIXME: we should calculate the absolute position and height of the textLines!
                my.drawText(
                    camera,
                    textLine.text,
                    x + textLine.position.x * sliceContainer.newLayout.absoluteScale,
                    y + textLine.position.y * sliceContainer.newLayout.absoluteScale + textLine.fontHeight * sliceContainer.newLayout.absoluteScale,
                    textLine.fontHeight * sliceContainer.newLayout.absoluteScale,
                    textLine.fontColor,
                    textLine.baseFont,
                    opacity
                )

            }

        }


        // TODO: maybe only draw if container body is drawn?
        if (ZUI.main.showGrid) {
            var xStartGrid = sliceContainer.newLayout.absolutePosition.x
            var yStartGrid = sliceContainer.newLayout.absolutePosition.y
            my.fillAndDrawRect(camera, xStartGrid - 5, yStartGrid, 10, 0, null, { r: 100, g: 100, b: 100, a: 1 }, 1, 0.2)
            my.fillAndDrawRect(camera, xStartGrid, yStartGrid - 5, 0, 10, null, { r: 100, g: 100, b: 100, a: 1 }, 1, 0.2)

            var horizontalGridDistance = 10 * sliceContainer.newLayout.absoluteScale; // FIXME
            var verticalGridDistance = 10 * sliceContainer.newLayout.absoluteScale; // FIXME

            var context2d = camera.context2d
            context2d.save()

            // clip rectangle
            {

                var widthPx = width * camera.pixelsPerMeter
                var heightPx = height * camera.pixelsPerMeter
                var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
                var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter - heightPx

                // Note: only drawing the portion that fits inside the camera!
                if (
                    leftPx <= camera.pixelPosition.leftPx + camera.pixelSize.widthPx &&
                    leftPx + widthPx >= camera.pixelPosition.leftPx &&
                    topPx <= camera.pixelPosition.topPx + camera.pixelSize.heightPx &&
                    topPx + heightPx >= camera.pixelPosition.topPx
                ) {

                    context2d.beginPath()
                    context2d.rect(leftPx, topPx, widthPx, heightPx)
                    context2d.clip()
                }

            }

            for (var horIndex = - Math.floor(width / horizontalGridDistance); horIndex < width / horizontalGridDistance; horIndex++) {
                my.fillAndDrawRect(camera, xStartGrid + horIndex * horizontalGridDistance, yStartGrid - height, 0.4, height * 2, null, { r: 100, g: 100, b: 100, a: 1 }, 0, 0.1)
            }
            for (var verIndex = - Math.floor(height / verticalGridDistance); verIndex < height / verticalGridDistance; verIndex++) {
                my.fillAndDrawRect(camera, xStartGrid - width, yStartGrid + verIndex * verticalGridDistance, width * 2, 0.4, null, { r: 100, g: 100, b: 100, a: 1 }, 0, 0.1)
            }

            context2d.restore()


        }


    }

    my.drawContainersNew = function (camera, currentSliceContainer, drawingAfterConnections) {
        var containerProperties = currentSliceContainer.worldContainer.containerProperties

        // TODO: also check if this container should not be drawn, but it's children might!
        if (currentSliceContainer.isVisible) {
            if (containerProperties.drawContainer) {
                my.drawContainerNew(camera, currentSliceContainer, drawingAfterConnections)
            }

            if (currentSliceContainer.hasOwnProperty('sliceChildren')) {
                for (var childIndex = 0; childIndex < currentSliceContainer.sliceChildren.length; childIndex++) {
                    var childSliceContainer = currentSliceContainer.sliceChildren[childIndex]

                    my.drawContainersNew(camera, childSliceContainer, drawingAfterConnections)
                }
            }
        }

    }

    my.drawConnections = function (camera, sliceConnections) {

        // TODO: there is a possible speed-up if many (separate) paths are send to the canvas before calling 'stroke()'
        //       it is however not possible to change line-width between these paths, so it's not obvious how to take advantage of this.

// var context2d = camera.context2d
// context2d.beginPath()
        var loopIndex
        for (loopIndex = 0; loopIndex < sliceConnections.length; loopIndex++) {
            var sliceConnection = sliceConnections[loopIndex]

            var connectionProperties = sliceConnection.worldConnection.connectionProperties

            if (sliceConnection.isVisible) {

                // FIXME: should use a gradient or use the average?
                var opacity = sliceConnection.opacity

                var fromAttachmentPoint = sliceConnection.fromAttachmentPoint

                var toLineEndPoint = sliceConnection.toLineEndPoint

                var toBendingPoint = sliceConnection.toBendingPoint
                var fromBendingPoint = sliceConnection.fromBendingPoint

                // - draw the connection line
                var lineWidth = 2
                if (connectionProperties.lineNormalizedWidth != null) {
                    // FIXME: should we really use the rootWorldSliceContainer of the camera like this?
                    lineWidth = connectionProperties.lineNormalizedWidth * (camera.rootWorldSliceContainer.size.height / camera.rootWorldSliceContainer.worldContainer.normalizedSize.height)
                }

                var lineColor = connectionProperties.arrowLineColor
/*
                var lineWidthPx = lineWidth * camera.pixelsPerMeter
                context2d.lineWidth = lineWidthPx
                context2d.strokeStyle = my.getCanvasRGBAColor(lineColor, opacity)
*/

                if (lineColor != null) {
                    my.drawBezierCurve(camera, fromAttachmentPoint.x, fromAttachmentPoint.y, fromBendingPoint.x, fromBendingPoint.y, toBendingPoint.x, toBendingPoint.y, toLineEndPoint.x, toLineEndPoint.y, lineColor, lineWidth, opacity)

                    var childIndex
                    for (childIndex = 0; childIndex < sliceConnection.sliceChildren.length; childIndex++) {
                        var sliceChildContainer = sliceConnection.sliceChildren[childIndex]

                        var containerProperties = sliceChildContainer.worldContainer.containerProperties

                        // FIXME: what to do with this?
                        var drawAfterConnections = containerProperties.drawAfterConnections

                        my.drawContainers(camera, sliceChildContainer, drawAfterConnections)
                    }

                }

/*
                if (loopIndex % 10 == 9) {
                    context2d.stroke()
                    context2d.beginPath()
                }
*/
            }
        }
// context2d.stroke()


        var loopIndex
        for (loopIndex = 0; loopIndex < sliceConnections.length; loopIndex++) {
            var sliceConnection = sliceConnections[loopIndex]

            var connectionProperties = sliceConnection.worldConnection.connectionProperties

            if (sliceConnection.isVisible) {

                // FIXME: should use a gradient or use the average?
                var opacity = sliceConnection.opacity

                var toAttachmentPoint = sliceConnection.toAttachmentPoint

                var toArrowPointLeftSide = sliceConnection.toArrowPointLeftSide
                var toArrowPointRightSide = sliceConnection.toArrowPointRightSide

                if (sliceConnection.showArrowHead) {
                    // - draw the arrowhead
                    var fillColor = connectionProperties.arrowHeadColor
                    my.drawArrowHead(camera, toAttachmentPoint.x, toAttachmentPoint.y, toArrowPointLeftSide.x, toArrowPointLeftSide.y, toArrowPointRightSide.x, toArrowPointRightSide.y, fillColor, opacity)
                }

            }
        }

    }

    my.drawGuide = function (camera, x, y, lineColor) {
        var context2d = camera.context2d

        // TODO: only draw the portion that fits inside the camera!

        var leftPx = camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2 + (x - camera.centerPosition.x) * camera.pixelsPerMeter
        var topPx = camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2 - (y - camera.centerPosition.y) * camera.pixelsPerMeter

//        context2d.fillStyle =

        context2d.beginPath()
        context2d.lineWidth = "1"
        context2d.strokeStyle = my.getCanvasRGBAColor(lineColor, 1.0)
        context2d.moveTo(leftPx - 5, topPx)
        context2d.lineTo(leftPx - 10, topPx)
        context2d.stroke()
        context2d.moveTo(leftPx + 5, topPx)
        context2d.lineTo(leftPx + 10, topPx)
        context2d.stroke()
        context2d.moveTo(leftPx, topPx + 5)
        context2d.lineTo(leftPx, topPx + 10)
        context2d.stroke()
        context2d.moveTo(leftPx, topPx - 5)
        context2d.lineTo(leftPx, topPx - 10)
        context2d.stroke()

    }

    my.drawGuides = function (camera, guidePoints, guideLines) {

        var loopIndex
        for (loopIndex = 0; loopIndex < guidePoints.length; loopIndex++) {
            var guidePoint = guidePoints[loopIndex]

            var container = guidePoint.container

            var guidePointPosition = {
                x: container.position.x + guidePoint.deltaX,
                y: container.position.y + guidePoint.deltaY
            }

            var guideColor = {r: 255, g: 0, b: 0, a: 1.0}

            my.drawGuide(camera, guidePointPosition.x, guidePointPosition.y, guideColor)

        }

    }

    return my
}()
