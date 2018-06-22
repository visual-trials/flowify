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

ZUI.canvas = function () {

    var my = {}

    my.canvasElement = document.getElementById("canvas")
    my.context2d = my.canvasElement.getContext("2d")

    my.fixedCanvasSize = null

    my.nrOfRedrawTimes = 0
    my.lastRedrawTimes = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    my.redrawTimeIndex = 0

    my.beforeRedrawCanvas = null
    my.afterRedrawCanvas = null

    my.logLines = []
    my.maxNrOfLogLines = 20
    my.logCamera = null

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

    my.clipCameraRectangle = function (camera) {
        // FIXME: we can't use this here, since it could be a pipelined conext! var context2d = camera.context2d
        var context2d = my.context2d

        context2d.beginPath()
        context2d.rect(
            camera.pixelPosition.leftPx,
            camera.pixelPosition.topPx,
            camera.pixelSize.widthPx,
            camera.pixelSize.heightPx
        )
        context2d.clip()
    }

    my.drawShadowNextToRectanglePx = function (rectanglePositionPx, rectangleSizePx, sideToDraw, nrOfPixels, startingAlpha) {

        if (sideToDraw === 'left') {
            var gradient = my.context2d.createLinearGradient(rectanglePositionPx.leftPx - nrOfPixels, rectanglePositionPx.topPx, rectanglePositionPx.leftPx, rectanglePositionPx.topPx);
            gradient.addColorStop(0, "rgba(0, 0, 0, 0)");
            gradient.addColorStop(1, "rgba(0, 0, 0, " + startingAlpha + ")");
            ZUI.canvas.context2d.fillStyle = gradient
            ZUI.canvas.context2d.fillRect(rectanglePositionPx.leftPx - nrOfPixels, rectanglePositionPx.topPx, nrOfPixels, rectangleSizePx.heightPx)
        }
        else if (sideToDraw === 'right') {
            var gradient = ZUI.canvas.context2d.createLinearGradient(rectanglePositionPx.leftPx + rectangleSizePx.widthPx, rectanglePositionPx.topPx, rectanglePositionPx.leftPx + rectangleSizePx.widthPx + nrOfPixels, rectanglePositionPx.topPx);
            gradient.addColorStop(0, "rgba(0, 0, 0, " + startingAlpha + ")");
            gradient.addColorStop(1, "rgba(0, 0, 0, 0)");
            ZUI.canvas.context2d.fillStyle = gradient
            ZUI.canvas.context2d.fillRect(rectanglePositionPx.leftPx + rectangleSizePx.widthPx, rectanglePositionPx.topPx, nrOfPixels, rectangleSizePx.heightPx)
        }
        else if (sideToDraw === 'top') {
            var gradient = ZUI.canvas.context2d.createLinearGradient(rectanglePositionPx.leftPx, rectanglePositionPx.topPx, rectanglePositionPx.leftPx, rectanglePositionPx.topPx + nrOfPixels);
            gradient.addColorStop(0, "rgba(0, 0, 0, 0)");
            gradient.addColorStop(1, "rgba(0, 0, 0, " + startingAlpha + ")");
            ZUI.canvas.context2d.fillStyle = gradient
            ZUI.canvas.context2d.fillRect(rectanglePositionPx.leftPx, rectanglePositionPx.topPx, rectangleSizePx.widthPx, nrOfPixels)
        }

        // FIXME: add more possible sides to draw!

    }

    my.splitRectanglePxHorizontallyUsingPercentage = function (rectanglePx, percentage) {

        var widthLeftRectanglePx = rectanglePx.pixelSize.widthPx * percentage

        return my.splitRectanglePxHorizontallyUsingPixels(rectanglePx, widthLeftRectanglePx)
    }

    my.splitRectanglePxHorizontallyUsingPixels = function (rectanglePx, widthLeftRectanglePx) {

        var leftRectanglePx = {
            pixelPosition: {
                leftPx: rectanglePx.pixelPosition.leftPx,
                topPx: rectanglePx.pixelPosition.topPx
            },
            pixelSize: {
                widthPx: widthLeftRectanglePx,
                heightPx: rectanglePx.pixelSize.heightPx
            }
        }

        var rightRectanglePx = {
            pixelPosition: {
                leftPx: rectanglePx.pixelPosition.leftPx + widthLeftRectanglePx,
                topPx: rectanglePx.pixelPosition.topPx
            },
            pixelSize: {
                widthPx: rectanglePx.pixelSize.widthPx - widthLeftRectanglePx,
                heightPx: rectanglePx.pixelSize.heightPx
            }
        }

        return { leftRectanglePx: leftRectanglePx, rightRectanglePx: rightRectanglePx}
    }

    my.splitRectanglePxVerticallyUsingPercentage = function (rectanglePx, percentage) {

        var heightTopRectanglePx = rectanglePx.pixelSize.heightPx * percentage

        return my.splitRectanglePxVerticallyUsingPixels(rectanglePx, heightTopRectanglePx)
    }

    my.splitRectanglePxVerticallyUsingPixels = function (rectanglePx, heightTopRectanglePx) {

        var topRectanglePx = {
            pixelPosition: {
                leftPx: rectanglePx.pixelPosition.leftPx,
                topPx: rectanglePx.pixelPosition.topPx
            },
            pixelSize: {
                widthPx: rectanglePx.pixelSize.widthPx,
                heightPx: heightTopRectanglePx
            }
        }

        var bottomRectanglePx = {
            pixelPosition: {
                leftPx: rectanglePx.pixelPosition.leftPx,
                topPx: rectanglePx.pixelPosition.topPx + heightTopRectanglePx
            },
            pixelSize: {
                widthPx: rectanglePx.pixelSize.widthPx,
                heightPx: rectanglePx.pixelSize.heightPx - heightTopRectanglePx
            }
        }

        return { topRectanglePx: topRectanglePx, bottomRectanglePx: bottomRectanglePx}
    }


    // Log

    my.log = function (logText) {
        my.logLines.push(logText)
        my.truncateLog()
    }

    my.truncateLog = function () {
        var logLines = my.logLines

        var newLogLines = []
        for (var logLineIndex = logLines.length - my.maxNrOfLogLines; logLineIndex < logLines.length; logLineIndex++) {
            var logLine = logLines[logLineIndex]
            newLogLines.push(logLine)
        }
        my.logLines = newLogLines
    }

    my.drawLog = function () {
        if (my.logCamera == null) {
            my.logCamera = {}
            my.logCamera.context2d = ZUI.canvas.context2d
        }

        var topTextPx = 20
        var logLines = my.logLines
        for (var logLineIndex = 0; logLineIndex < logLines.length; logLineIndex++) {
            var logLine = logLines[logLineIndex]
            if (logLine != null) {
                topTextPx = ZUI.render.drawTextPx(my.logCamera, logLine, "rgba(0, 0, 0, 1)", 10, topTextPx, 1000, 20)
            }
        }
    }

    // Redraw Timing

    my.startRedrawTiming = function () {
        my.beforeRedrawCanvas = Date.now()
    }

    my.endRedrawTiming = function () {
        my.afterRedrawCanvas = Date.now()
    }


    my.logRedrawTimes = function () {
        var lastRedrawTime = my.afterRedrawCanvas - my.beforeRedrawCanvas
        my.lastRedrawTimes[my.redrawTimeIndex] = lastRedrawTime
        if (my.nrOfRedrawTimes < 30) {
            my.nrOfRedrawTimes += 1
        }
        var avgRedrawTime = 0
        for (var i = 0; i < my.nrOfRedrawTimes; i++) {
            avgRedrawTime += my.lastRedrawTimes[i]
        }
        avgRedrawTime = avgRedrawTime / my.nrOfRedrawTimes

        ZUI.log(lastRedrawTime + ' (' + avgRedrawTime + ')')

        if (my.redrawTimeIndex > 30) {
            my.redrawTimeIndex = 0
        }
        else {
            my.redrawTimeIndex += 1
        }
    }

    my.drawRedrawTimes = function () {
        var lastRedrawTime = my.afterRedrawCanvas - my.beforeRedrawCanvas
        my.lastRedrawTimes[my.redrawTimeIndex] = lastRedrawTime
        if (my.nrOfRedrawTimes < 30) {
            my.nrOfRedrawTimes += 1
        }
        var avgRedrawTime = 0
        for (var i = 0; i < my.nrOfRedrawTimes; i++) {
            avgRedrawTime += my.lastRedrawTimes[i]
        }
        avgRedrawTime = avgRedrawTime / my.nrOfRedrawTimes

        camera = {}
        camera.context2d = my.context2d
        camera.pixelSize = {
            widthPx: 300,
            heightPx: 100
        }
        camera.pixelPosition = {
            leftPx: 100,
            topPx: my.canvasElement.height - camera.pixelSize.heightPx - 50
        }

        ZUI.render.drawBarGraphPx(camera, my.lastRedrawTimes, 30, my.redrawTimeIndex, 33)

        /*
        ZUI.log(lastRedrawTime + ' (' + avgRedrawTime + ')')
        */

        if (my.redrawTimeIndex > 30) {
            my.redrawTimeIndex = 0
        }
        else {
            my.redrawTimeIndex += 1
        }
    }

    my.pipelinedContext2d = {}

    my.pipelinedContext2d.doLogging = false

    my.pipelinedContext2d.reset = function () {
        my.pipelinedContext2d.commands = []
        my.pipelinedContext2d.commandSets = []
        my.pipelinedContext2d.commandSetsBySetIdentifier = {}
        my.pipelinedContext2d.currentCommand = {}
        my.pipelinedContext2d.lastCommand = null
        my.pipelinedContext2d.currentSubCommands = []
        my.pipelinedContext2d.justFilledOrStroked = false

        my.pipelinedContext2d.font = null
        my.pipelinedContext2d.fillStyle = null
        my.pipelinedContext2d.strokeStyle = null
        my.pipelinedContext2d.lineWidth = null
    }

    my.pipelinedContext2d.beginPath = function () {
        my.pipelinedContext2d.currentSubCommands = []
        my.pipelinedContext2d.justFilledOrStroked = false
    }

    my.pipelinedContext2d.rect = function (leftPx, topPx, widthPx, heightPx) {
        my.pipelinedContext2d.currentSubCommands.push({
            "cmd": "rect",
            "leftPx": leftPx,
            "topPx": topPx,
            "widthPx": widthPx,
            "heightPx": heightPx
        })
        my.pipelinedContext2d.currentCommand.commandType = 'rect'
        my.pipelinedContext2d.justFilledOrStroked = false
    }

    my.pipelinedContext2d.ellipse = function (leftPx, topPx, widthPx, heightPx, dummy1, dummy2, dummy3) {
        my.pipelinedContext2d.currentSubCommands.push({
            "cmd": "ellipse",
            "leftPx": leftPx,
            "topPx": topPx,
            "widthPx": widthPx,
            "heightPx": heightPx
        })
        my.pipelinedContext2d.currentCommand.commandType = 'ellipse'
        my.pipelinedContext2d.justFilledOrStroked = false
    }

    my.pipelinedContext2d.bezierCurveTo = function (fromBendingLeftPx, fromBendingTopPx, toBendingLeftPx, toBendingTopPx, toLeftPx, toTopPx, lineWidthPx) {
        my.pipelinedContext2d.currentSubCommands.push({
            "cmd": "bezierCurveTo",
            "fromBendingLeftPx": fromBendingLeftPx,
            "fromBendingTopPx": fromBendingTopPx,
            "toBendingLeftPx": toBendingLeftPx,
            "toBendingTopPx": toBendingTopPx,
            "toLeftPx": toLeftPx,
            "toTopPx": toTopPx,
            "lineWidthPx": lineWidthPx // FIXME: this is redundant, since it is also given as an ctx-property?
        })
        my.pipelinedContext2d.currentCommand.commandType = 'line'
        my.pipelinedContext2d.justFilledOrStroked = false
    }

    my.pipelinedContext2d.moveTo = function (leftPx, topPx) {
        my.pipelinedContext2d.currentSubCommands.push({
            "cmd": "moveTo",
            "leftPx": leftPx,
            "topPx": topPx
        })
        my.pipelinedContext2d.justFilledOrStroked = false
    }

    my.pipelinedContext2d.lineTo = function (leftPx, topPx) {
        my.pipelinedContext2d.currentSubCommands.push({
            "cmd": "lineTo",
            "leftPx": leftPx,
            "topPx": topPx
        })
        my.pipelinedContext2d.currentCommand.commandType = 'line'
        my.pipelinedContext2d.justFilledOrStroked = false
    }

    my.pipelinedContext2d.closePath = function () {
        my.pipelinedContext2d.currentSubCommands.push({
            "cmd": "closePath"
        })
        my.pipelinedContext2d.justFilledOrStroked = false
    }

    my.pipelinedContext2d.fill = function () {
        var command = null

        if (!my.pipelinedContext2d.justFilledOrStroked) {
            command = my.pipelinedContext2d.currentCommand
            command.fillOrStroke = 'fill'
            command.subCommands = my.pipelinedContext2d.currentSubCommands
            my.pipelinedContext2d.commands.push(command)
            my.pipelinedContext2d.justFilledOrStroked = true

            my.pipelinedContext2d.lastCommand = command
            // TODO: redundant, this is also done in beginPath!
            my.pipelinedContext2d.currentSubCommands = []
            my.pipelinedContext2d.currentCommand = {}
        }
        else {
            my.pipelinedContext2d.lastCommand.fillOrStroke = 'fillAndStroke'
            my.pipelinedContext2d.justFilledOrStroked = false
        }

        command.fillStyle = my.pipelinedContext2d.fillStyle
    }

    my.pipelinedContext2d.stroke = function () {
        var command = null

        if (!my.pipelinedContext2d.justFilledOrStroked) {
            command = my.pipelinedContext2d.currentCommand
            command.fillOrStroke = 'stroke'
            command.subCommands = my.pipelinedContext2d.currentSubCommands
            my.pipelinedContext2d.commands.push(command)
            my.pipelinedContext2d.justFilledOrStroked = true

            my.pipelinedContext2d.lastCommand = command
            // TODO: redundant, this is also done in beginPath!
            my.pipelinedContext2d.currentSubCommands = []
            my.pipelinedContext2d.currentCommand = {}
        }
        else {
            command = my.pipelinedContext2d.lastCommand
            command.fillOrStroke = 'fillAndStroke'
            my.pipelinedContext2d.justFilledOrStroked = false
        }

        command.strokeStyle = my.pipelinedContext2d.strokeStyle
        command.lineWidth = my.pipelinedContext2d.lineWidth
    }

    my.pipelinedContext2d.fillText = function (text, leftPx, topPx) {
        my.pipelinedContext2d.currentSubCommands.push({
            "cmd": "fillText",
            "text": text,
            "leftPx": leftPx,
            "topPx": topPx
        })
        my.pipelinedContext2d.justFilledOrStroked = false

        var command = my.pipelinedContext2d.currentCommand
        command.commandType = 'text'
        command.fillOrStroke = 'fill'
        command.font = my.pipelinedContext2d.font
        command.fillStyle = my.pipelinedContext2d.fillStyle
        command.subCommands = my.pipelinedContext2d.currentSubCommands
        my.pipelinedContext2d.commands.push(command)

        // TODO: redundant, this is also done in beginPath!
        my.pipelinedContext2d.currentSubCommands = []
        my.pipelinedContext2d.currentCommand = {}
    }

    my.pipelinedContext2d.addCommandAsSet = function (command) {

        my.pipelinedContext2d.commandSets.push([command])
        // my.pipelinedContext2d.commandSetsBySetIdentifier[commandSetIdentifier] = newCommandSet

    }

    my.pipelinedContext2d.addCommandToSets = function (command) {
        var commandSetIdentifier = 'all'

        if (command.commandType === 'text') {
            commandSetIdentifier = command.commandType + command.fillOrStroke + command.fillStyle + command.font
        }
        else if (command.commandType === 'rect') {
            if (command.fillOrStroke === 'stroke') {
                commandSetIdentifier = command.commandType + command.fillOrStroke + command.strokeStyle + command.lineWidth
            }
            else if (command.fillOrStroke === 'fill') {
                commandSetIdentifier = command.commandType + command.fillOrStroke + command.fillStyle
            }
            else if (command.fillOrStroke === 'fillAndStroke') {
                commandSetIdentifier = command.commandType + command.fillOrStroke + command.fillStyle + command.strokeStyle + command.lineWidth
            }
            else {
                ZUI.log("Unknown fillOrStroke command: ",  command.fillOrStroke)
            }
        }
        else if (command.commandType === 'ellipse') {
            if (command.fillOrStroke === 'stroke') {
                commandSetIdentifier = command.commandType + command.fillOrStroke + command.strokeStyle + command.lineWidth
            }
            else if (command.fillOrStroke === 'fill') {
                commandSetIdentifier = command.commandType + command.fillOrStroke + command.fillStyle
            }
            else if (command.fillOrStroke === 'fillAndStroke') {
                commandSetIdentifier = command.commandType + command.fillOrStroke + command.fillStyle + command.strokeStyle + command.lineWidth
            }
            else {
                ZUI.log("Unknown fillOrStroke command: ",  command.fillOrStroke)
            }
        }
        else if (command.commandType === 'line') {
            if (command.fillOrStroke === 'stroke') {
                commandSetIdentifier = command.commandType + command.fillOrStroke + command.strokeStyle + command.lineWidth
            }
            else if (command.fillOrStroke === 'fill') {
                commandSetIdentifier = command.commandType + command.fillOrStroke + command.fillStyle
            }
            else if (command.fillOrStroke === 'fillAndStroke') {
                commandSetIdentifier = command.commandType + command.fillOrStroke + command.fillStyle + command.strokeStyle + command.lineWidth
            }
            else {
                ZUI.log("Unknown fillOrStroke command: ",  command.fillOrStroke)
            }
        }
        else {
            ZUI.log("Unknown commandType:" + command.commandType)
        }

        var commandSetToAddCommandTo = null
        if (my.pipelinedContext2d.commandSetsBySetIdentifier.hasOwnProperty(commandSetIdentifier)) {
            commandSetToAddCommandTo = my.pipelinedContext2d.commandSetsBySetIdentifier[commandSetIdentifier]
        }
        else {
            var newCommandSet = []

            // FIXME: we should insert the newCommandSet in-between the other commandSets depending on the z-layer!
            my.pipelinedContext2d.commandSets.push(newCommandSet)
            my.pipelinedContext2d.commandSetsBySetIdentifier[commandSetIdentifier] = newCommandSet

            commandSetToAddCommandTo = newCommandSet
        }

        commandSetToAddCommandTo.push(command)
    }

    my.pipelinedContext2d.executeCommands = function () {

        for (var commandIndex = 0; commandIndex < my.pipelinedContext2d.commands.length; commandIndex++) {
            var command = my.pipelinedContext2d.commands[commandIndex]

            my.pipelinedContext2d.addCommandToSets(command)
            // my.pipelinedContext2d.addCommandAsSet(command)
        }

        for (var commandSetIndex = 0; commandSetIndex < my.pipelinedContext2d.commandSets.length; commandSetIndex++) {
            var commandSet = my.pipelinedContext2d.commandSets[commandSetIndex]

            my.pipelinedContext2d.executeCommandSet(commandSet)
        }
    }

    my.pipelinedContext2d.executeCommandSet = function (commandSet) {
        var context2d = ZUI.canvas.context2d

        // FIXME: we should not do this, instead put the information in the commandSet itself!
        var firstCommandInSet = commandSet[0]

        if (firstCommandInSet.commandType === 'rect') {
            context2d.beginPath()

            for (var commandIndexInSet = 0; commandIndexInSet < commandSet.length; commandIndexInSet++) {
                var command = commandSet[commandIndexInSet]

                for (var subCommandIndex = 0; subCommandIndex < command.subCommands.length; subCommandIndex++) {
                    var subCommand = command.subCommands[subCommandIndex]

                    context2d.rect(subCommand.leftPx, subCommand.topPx, subCommand.widthPx, subCommand.heightPx)
                }
            }

            if (firstCommandInSet.fillOrStroke === 'fill' || firstCommandInSet.fillOrStroke === 'fillAndStroke') {
                context2d.fillStyle = firstCommandInSet.fillStyle
                context2d.fill()
            }
            if (firstCommandInSet.fillOrStroke === 'stroke' || firstCommandInSet.fillOrStroke === 'fillAndStroke') {
                context2d.strokeStyle = firstCommandInSet.strokeStyle
                context2d.lineWidth = firstCommandInSet.lineWidth
                context2d.stroke()
            }
        }
        else if (firstCommandInSet.commandType === 'ellipse') {
            context2d.beginPath()

            for (var commandIndexInSet = 0; commandIndexInSet < commandSet.length; commandIndexInSet++) {
                var command = commandSet[commandIndexInSet]

                for (var subCommandIndex = 0; subCommandIndex < command.subCommands.length; subCommandIndex++) {
                    var subCommand = command.subCommands[subCommandIndex]

                    if (ZUI.canvas.isIE) {
                        // FIXME: drawEllipseIE(context2d, leftPx, topPx, widthPx, heightPx)
                    }
                    else {
                        // FIXME: maybe use arc?
                        context2d.moveTo(subCommand.leftPx + subCommand.widthPx, subCommand.topPx)
                        context2d.ellipse(subCommand.leftPx, subCommand.topPx, subCommand.widthPx, subCommand.heightPx, 0, 0, 2 * Math.PI);
                    }
                }
            }

            if (firstCommandInSet.fillOrStroke === 'fill' || firstCommandInSet.fillOrStroke === 'fillAndStroke') {
                context2d.fillStyle = firstCommandInSet.fillStyle
                context2d.fill()
            }
            if (firstCommandInSet.fillOrStroke === 'stroke' || firstCommandInSet.fillOrStroke === 'fillAndStroke') {
                context2d.strokeStyle = firstCommandInSet.strokeStyle
                context2d.lineWidth = firstCommandInSet.lineWidth
                context2d.stroke()
            }

        }
        else if (firstCommandInSet.commandType === 'line') {
            context2d.beginPath()

            for (var commandIndexInSet = 0; commandIndexInSet < commandSet.length; commandIndexInSet++) {
                var command = commandSet[commandIndexInSet]

                for (var subCommandIndex = 0; subCommandIndex < command.subCommands.length; subCommandIndex++) {
                    var subCommand = command.subCommands[subCommandIndex]

                    if (subCommand.cmd === 'moveTo') {
                        context2d.moveTo(subCommand.leftPx, subCommand.topPx)
                    }
                    else if (subCommand.cmd === 'lineTo') {
                        context2d.lineTo(subCommand.leftPx, subCommand.topPx)
                    }
                    else if (subCommand.cmd === 'bezierCurveTo') {
                        context2d.bezierCurveTo(subCommand.fromBendingLeftPx, subCommand.fromBendingTopPx, subCommand.toBendingLeftPx, subCommand.toBendingTopPx, subCommand.toLeftPx, subCommand.toTopPx)
                    }
                    else if (subCommand.cmd === 'closePath') {
                        context2d.closePath()
                    }
                }

            }

            if (firstCommandInSet.fillOrStroke === 'fill' || firstCommandInSet.fillOrStroke === 'fillAndStroke') {
                context2d.fillStyle = firstCommandInSet.fillStyle
                context2d.fill()
            }
            if (firstCommandInSet.fillOrStroke === 'stroke' || firstCommandInSet.fillOrStroke === 'fillAndStroke') {
                context2d.strokeStyle = firstCommandInSet.strokeStyle
                context2d.lineWidth = firstCommandInSet.lineWidth
                context2d.stroke()
            }
        }
        else {
            for (var commandIndexInSet = 0; commandIndexInSet < commandSet.length; commandIndexInSet++) {
                var command = commandSet[commandIndexInSet]

                if (command.commandType === 'text') {

                    context2d.font = command.font
                    context2d.fillStyle = command.fillStyle

                    for (var subCommandIndex = 0; subCommandIndex < command.subCommands.length; subCommandIndex++) {
                        var subCommand = command.subCommands[subCommandIndex]
                        context2d.fillText(subCommand.text, subCommand.leftPx, subCommand.topPx)
                    }

                }
            }
        }


    }

    return my
}()
