/*

   Copyright 2016-2018 Jeffrey Hullekes

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

ZUI.layout = function () {

    var my = {}

    my.characterWidths = {}
    
    my.measureTextWidth = function (text, fontHeight, baseFont) {
        var context2d = ZUI.canvas.context2d  // TODO: we should be able to measureText without a canvas!

        var pixelsPerMeter = 1  // Note: since we are not interested in in absolute measurement, but just the relation between height and width, we set this to 1

        // Note: fontHeight is the distance between the top of a capital and the bottom of a capital (note that a non-capital can stick out below!)
        var fontHeight2FontPx = 1.3  // TODO: is this multiplier correct?
        var fontPx = Math.round(fontHeight * fontHeight2FontPx * pixelsPerMeter)

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

    my.doSizingPositioningAndScaling = function (world, currentSliceContainer) {

        var containerProperties = currentSliceContainer.worldContainer.containerProperties

        var hasPadding = containerProperties.hasPadding  // FIXME: this if not used atm!
        var paddingLeft = containerProperties.paddingLeft
        var paddingRight = containerProperties.paddingRight
        var paddingTop = containerProperties.paddingTop
        var paddingBottom = containerProperties.paddingBottom
        var paddingBetweenChildren = containerProperties.paddingBetweenChildren
        var childrenLayoutFunction = containerProperties.childrenLayoutFunction
        
        // FIXME: should we set the newLayout.relativeScale here? or at each layout function?
        var relativeScale = 1
        if (currentSliceContainer.worldContainer.manualRelativeScale != null) {
            relativeScale = currentSliceContainer.worldContainer.manualRelativeScale
        }
        currentSliceContainer.newLayout.relativeScale = relativeScale
            

        // FIXME: set isVisible dynamically
        currentSliceContainer.isVisible = true

        if (childrenLayoutFunction === 'verticalTopToBottom') {

            // This is a topToBottom layout function

            var largestChildWidth = 0
            var sumChildrenHeight = 0
            var oneOrMoreChildrenAdded = false

            for (var loopIndex = 0; loopIndex < currentSliceContainer.sliceChildren.length; loopIndex++) {
                var childSliceContainer = currentSliceContainer.sliceChildren[loopIndex]

                var childSize = my.doSizingPositioningAndScaling(world, childSliceContainer)

                if (childSize != null) {

                    if (childSize.width > largestChildWidth) {
                        largestChildWidth = childSize.width
                    }

                    if (oneOrMoreChildrenAdded) {
                        sumChildrenHeight += paddingBetweenChildren
                    }

                    childSliceContainer.newLayout.position.x = paddingLeft
                    childSliceContainer.newLayout.position.y = - paddingTop - sumChildrenHeight
                    childSliceContainer.newLayout.isPositionOf = 'left-top'
                    childSliceContainer.newLayout.positionOriginatesFrom = 'left-top'

                    sumChildrenHeight += childSize.height
                    oneOrMoreChildrenAdded = true
                }

            }

            // FIXME: HACK to enlarge the childs to the maxWidth of the other childs! (among other things, this should keep in mind the pointsTo etc, and re-center text/containers in the child!)
            for (var loopIndex = 0; loopIndex < currentSliceContainer.sliceChildren.length; loopIndex++) {
                var childSliceContainer = currentSliceContainer.sliceChildren[loopIndex]

                childSliceContainer.newLayout.size.width = largestChildWidth
            }

            currentSliceContainer.newLayout.size.width = paddingLeft + largestChildWidth + paddingRight
            currentSliceContainer.newLayout.size.height = paddingTop + sumChildrenHeight + paddingBottom


        }
        else if (childrenLayoutFunction === 'horizontalLeftToRight') {

            // This is a leftToRight layout function

            var largestChildHeight = 0
            var sumChildrenWidth = 0
            var oneOrMoreChildrenAdded = false

            for (var loopIndex = 0; loopIndex < currentSliceContainer.sliceChildren.length; loopIndex++) {
                var childSliceContainer = currentSliceContainer.sliceChildren[loopIndex]

                // TODO: we are currently re-doing the maxSize-ing. This shouldn't be necessary, but since we enlarge afterwards (with a HACK) we recompute the maxSize of each child here
                var childSize = my.doSizingPositioningAndScaling(world, childSliceContainer)

                if (childSize != null) {

                    if (childSize.height > largestChildHeight) {
                        largestChildHeight = childSize.height
                    }

                    if (oneOrMoreChildrenAdded) {
                        sumChildrenWidth += paddingBetweenChildren
                    }

                    childSliceContainer.newLayout.position.x = paddingLeft + sumChildrenWidth
                    childSliceContainer.newLayout.position.y = - paddingTop
                    childSliceContainer.newLayout.isPositionOf = 'left-top'
                    childSliceContainer.newLayout.positionOriginatesFrom = 'left-top'

                    sumChildrenWidth += childSize.width
                    oneOrMoreChildrenAdded = true

                }
            }

            currentSliceContainer.newLayout.size.width = paddingLeft + sumChildrenWidth + paddingRight
            currentSliceContainer.newLayout.size.height = paddingTop + largestChildHeight + paddingBottom


            return { width: currentSliceContainer.newLayout.size.width * currentSliceContainer.newLayout.relativeScale,
                height: currentSliceContainer.newLayout.size.height * currentSliceContainer.newLayout.relativeScale }
        }
        else if (childrenLayoutFunction === 'manualPositioning') {

            // This is a 'manualPosition' layout function

            var currentWidth = null
            var currentHeight = null

            if (currentSliceContainer.worldContainer.manualSize.width != null) {
                currentWidth = currentSliceContainer.worldContainer.manualSize.width  // FIXME: right now the manualSize is interpreted as the maxSize. Shouldn't we make this explicit?
            }

            if (currentSliceContainer.worldContainer.manualSize.height != null) {
                currentHeight = currentSliceContainer.worldContainer.manualSize.height  // FIXME: right now the manualSize is interpreted as the maxSize. Shouldn't we make this explicit?
            }

            for (var loopIndex = 0; loopIndex < currentSliceContainer.sliceChildren.length; loopIndex++) {
                var childSliceContainer = currentSliceContainer.sliceChildren[loopIndex]

                var childSize = my.doSizingPositioningAndScaling(world, childSliceContainer)

                if (childSize != null) {
                    var positionMultiplier = 1.0

                    // FIXME: we are assuming the x and y are always filled here! We need some check!
                    childSliceContainer.newLayout.position.x = childSliceContainer.worldContainer.manualPosition.x * positionMultiplier
                    childSliceContainer.newLayout.position.y = childSliceContainer.worldContainer.manualPosition.y * positionMultiplier

                    childSliceContainer.newLayout.isPositionOf = 'left-bottom'  // TODO: should this default be set here?
                    if (childSliceContainer.worldContainer.manualPosition.isPositionOf != null) {
                        childSliceContainer.newLayout.isPositionOf = childSliceContainer.worldContainer.manualPosition.isPositionOf
                    }
                    childSliceContainer.newLayout.positionOriginatesFrom = 'left-bottom'  // TODO: should this default be set here?
                    if (childSliceContainer.worldContainer.manualPosition.positionOriginatesFrom != null) {
                        childSliceContainer.newLayout.positionOriginatesFrom = childSliceContainer.worldContainer.manualPosition.positionOriginatesFrom
                    }

                }
            }

            currentSliceContainer.newLayout.size.width = currentWidth
            currentSliceContainer.newLayout.size.height = currentHeight


        }
        else if (childrenLayoutFunction === 'none') {

            // This is a one-textLine layout function

            // FIXME: check if containerText should be shown. But if not, then size of 0?? Or check for icons aswell?
            //  if (showContainerText)

            var containerText = containerProperties.containerText
            var containerTextFontHeight = containerProperties.containerTextFontHeight
            var containerTextColor = containerProperties.containerTextColor
            var containerTextFont = containerProperties.containerTextFont

            currentSliceContainer.textLines = []
            // FIXME: we are assuming only ONE text line here!
            // FIXME: should we add character-size-and-position information here? Or should we leave that to the renderer and the camera (for mouse-selecting-text)?
            var textLine = '' + containerText // TODO: forcing it to a string here. Is there a better solution?
            var textPosition = {
                x: paddingLeft,
                y: paddingBottom
            }
            var textLineWidth = my.measureTextWidth(textLine, containerTextFontHeight, containerTextFont)
            currentSliceContainer.textLines.push({
                text: containerText,
                baseFont: containerTextFont,
                fontColor: containerTextColor,
                fontHeight: containerTextFontHeight,
                textWidth: textLineWidth,
                position: textPosition
                // align: "TODO"
            })
            // TODO: isPositionOf and/or positionOriginatesFrom?

            // TODO: var textPaddingBetween/lineDistance for multilines

            currentSliceContainer.newLayout.size.width = paddingLeft + textLineWidth + paddingRight
            currentSliceContainer.newLayout.size.height = paddingTop + containerTextFontHeight + paddingBottom

            return { width: currentSliceContainer.newLayout.size.width * currentSliceContainer.newLayout.relativeScale,
                     height: currentSliceContainer.newLayout.size.height * currentSliceContainer.newLayout.relativeScale }
        }

        // TODO: shoulw we always return the size (* relativeScale) here?
        return { width: currentSliceContainer.newLayout.size.width * currentSliceContainer.newLayout.relativeScale,
                 height: currentSliceContainer.newLayout.size.height * currentSliceContainer.newLayout.relativeScale }

    }

    my.setAbsolutePositions = function (childRelativePosition, parentPosition) {
        var childPosition = {}

        childPosition.x = parentPosition.x + childRelativePosition.x
        childPosition.y = parentPosition.y + childRelativePosition.y

        return childPosition
    }

    my.findPositionInShape = function (knownPosition, isKnownPositionOf, knownShape, knownSize, whichPositionToFind) {

        var centerPosition = { x: 0, y: 0}
        var foundPosition = { x: 0, y: 0}

        // For any knownShape:
        // We FIRST calculate the centerPosition given the knownPosition,  isKnownPositionOf   and knownSize
        // We THEN  calculate the foundPosition  given the centerPosition, whichPositionToFind and knownSize

        if (knownShape === 'rectangle' || knownShape === 'ellipse') {  // FIXME: shouldn't we handle ellipse in a different way?

            // Options: left-top,       middle-top,     right-top,
            //          left-middle,    middle-middle,  right-middle,          // Note: middle-middle = center
            //          left-bottom,    middle-bottom,  right-bottom,

            if (isKnownPositionOf === 'left-bottom') {
                centerPosition.x = knownPosition.x + knownSize.width / 2
                centerPosition.y = knownPosition.y + knownSize.height / 2
            }
            else if (isKnownPositionOf === 'right-bottom') {
                centerPosition.x = knownPosition.x - knownSize.width / 2
                centerPosition.y = knownPosition.y + knownSize.height / 2
            }
            else if (isKnownPositionOf === 'middle-bottom') {
                centerPosition.x = knownPosition.x
                centerPosition.y = knownPosition.y + knownSize.height / 2
            }
            else if (isKnownPositionOf === 'left-middle') {
                centerPosition.x = knownPosition.x + knownSize.width / 2
                centerPosition.y = knownPosition.y
            }
            else if (isKnownPositionOf === 'right-middle') {
                centerPosition.x = knownPosition.x - knownSize.width / 2
                centerPosition.y = knownPosition.y
            }
            else if (isKnownPositionOf === 'middle-middle' || isKnownPositionOf === 'center') {
                centerPosition.x = knownPosition.x
                centerPosition.y = knownPosition.y
            }
            else if (isKnownPositionOf === 'left-top') {
                centerPosition.x = knownPosition.x + knownSize.width / 2
                centerPosition.y = knownPosition.y - knownSize.height / 2
            }
            else if (isKnownPositionOf === 'right-top') {
                centerPosition.x = knownPosition.x - knownSize.width / 2
                centerPosition.y = knownPosition.y - knownSize.height / 2
            }
            else if (isKnownPositionOf === 'middle-top') {
                centerPosition.x = knownPosition.x
                centerPosition.y = knownPosition.y - knownSize.height / 2
            }
            else {
                console.log("Unknown isKnownPositionOf: " + isKnownPositionOf)
            }

            // Options: left-top,       middle-top,     right-top,
            //          left-middle,    middle-middle,  right-middle,          // Note: middle-middle = center
            //          left-bottom,    middle-bottom,  right-bottom,

            if (whichPositionToFind === 'left-bottom') {
                foundPosition.x = centerPosition.x - knownSize.width / 2
                foundPosition.y = centerPosition.y - knownSize.height / 2
            }
            else if (whichPositionToFind === 'right-bottom') {
                foundPosition.x = centerPosition.x + knownSize.width / 2
                foundPosition.y = centerPosition.y - knownSize.height / 2
            }
            else if (whichPositionToFind === 'middle-bottom') {
                foundPosition.x = centerPosition.x
                foundPosition.y = centerPosition.y - knownSize.height / 2
            }
            else if (whichPositionToFind === 'left-middle') {
                foundPosition.x = centerPosition.x - knownSize.width / 2
                foundPosition.y = centerPosition.y
            }
            else if (whichPositionToFind === 'right-middle') {
                foundPosition.x = centerPosition.x + knownSize.width / 2
                foundPosition.y = centerPosition.y
            }
            else if (whichPositionToFind === 'middle-middle' || whichPositionToFind === 'center') {
                foundPosition.x = centerPosition.x
                foundPosition.y = centerPosition.y
            }
            else if (whichPositionToFind === 'left-top') {
                foundPosition.x = centerPosition.x - knownSize.width / 2
                foundPosition.y = centerPosition.y + knownSize.height / 2
            }
            else if (whichPositionToFind === 'right-top') {
                foundPosition.x = centerPosition.x + knownSize.width / 2
                foundPosition.y = centerPosition.y + knownSize.height / 2
            }
            else if (whichPositionToFind === 'middle-top') {
                foundPosition.x = centerPosition.x
                foundPosition.y = centerPosition.y + knownSize.height / 2
            }
            else {
                console.log("Unknown whichPositionToFind: " + whichPositionToFind)
            }

        }
        else {
            console.log("Unknown shape: " + knownShape)
        }

        return foundPosition
    }

    my.setAbsoluteContainerPositionsNew = function (parentAbsolutePosition, parentAbsoluteScale, parentSliceContainer, childSliceContainer) {

        childSliceContainer.newLayout.absoluteScale = parentAbsoluteScale * childSliceContainer.newLayout.relativeScale

        childSliceContainer.newLayout.absoluteSize.width = childSliceContainer.newLayout.size.width * childSliceContainer.newLayout.absoluteScale
        childSliceContainer.newLayout.absoluteSize.height = childSliceContainer.newLayout.size.height * childSliceContainer.newLayout.absoluteScale

        var childAbsoluteOriginPosition = {}
        if (parentSliceContainer != null) {
            // FIXME: hardcoded shape!
            childAbsoluteOriginPosition = my.findPositionInShape(parentAbsolutePosition, parentSliceContainer.newLayout.isPositionOf, 'rectangle', parentSliceContainer.newLayout.absoluteSize, childSliceContainer.newLayout.positionOriginatesFrom)
        }
        else {
            // if parentSliceContainer == null, we take over the parentAbsolutePosition for now (this should happen only once for the root container, right?)
            childAbsoluteOriginPosition.x = parentAbsolutePosition.x
            childAbsoluteOriginPosition.y = parentAbsolutePosition.y
        }

        childSliceContainer.newLayout.absolutePosition.x = childAbsoluteOriginPosition.x + childSliceContainer.newLayout.position.x * parentAbsoluteScale
        childSliceContainer.newLayout.absolutePosition.y = childAbsoluteOriginPosition.y + childSliceContainer.newLayout.position.y * parentAbsoluteScale

        if (childSliceContainer.hasOwnProperty('sliceChildren')) {
            for (var childIndex = 0; childIndex < childSliceContainer.sliceChildren.length; childIndex++) {
                var childOfChildSliceContainer = childSliceContainer.sliceChildren[childIndex]

                my.setAbsoluteContainerPositionsNew(childSliceContainer.newLayout.absolutePosition, childSliceContainer.newLayout.absoluteScale, childSliceContainer, childOfChildSliceContainer)
            }
        }

    }

    my.getContainerAttachmentPointByAngle = function (sliceContainer, angle) {

        var containerProperties = sliceContainer.worldContainer.containerProperties

        var attachmentPoint = {
            "x": null,
            "y": null
        }

        var containerAbsolutePosition = {}
        containerAbsolutePosition.x = sliceContainer.newLayout.absolutePosition.x
        containerAbsolutePosition.y = sliceContainer.newLayout.absolutePosition.y

        var containerAbsoluteSize = sliceContainer.newLayout.absoluteSize

        var centerPosition = my.findPositionInShape(containerAbsolutePosition, sliceContainer.newLayout.isPositionOf, containerProperties.shape, containerAbsoluteSize, 'center')

        var centerX = centerPosition.x
        var centerY = centerPosition.y

        if (containerProperties.shape === "ellipse") {
            // Using info from here:
            // http://stackoverflow.com/questions/11309596/how-to-get-a-point-on-an-ellipses-outline-given-an-angle
            // http://math.stackexchange.com/questions/22064/calculating-a-point-that-lies-on-an-ellipse-given-an-angle

            var tangentOfAngle = Math.tan(angle);
            var cosineOfAngle = Math.cos(angle);

            var multipliedTangent = tangentOfAngle * width / height
            var d = 1 / Math.sqrt(1 + multipliedTangent * multipliedTangent)

            if (cosineOfAngle > 0) {
                attachmentPoint.x = centerX + (width / 2) * d
                attachmentPoint.y = centerY + (height / 2) * d * multipliedTangent
            }
            else {
                attachmentPoint.x = centerX - (width / 2) * d
                attachmentPoint.y = centerY - (height / 2) * d * multipliedTangent
            }

        }
        else { // assuming a rectangle
            // Using info from here: http://stackoverflow.com/questions/4061576/finding-points-on-a-rectangle-at-a-given-angle
            // Also see: http://www.mathwords.com/t/t_assets/t28.gif

            var rectangleAngle = Math.atan2(height, width);
            var tangentOfAngle = Math.tan(angle);

            // Determining on which side of the rectangle the attachment-point lies

            var rightTopCornerAngle = rectangleAngle
            var rightBottomCornerAngle = -rectangleAngle
            var leftTopCornerAngle = Math.PI - rectangleAngle
            var leftBottomCornerAngle = rectangleAngle - Math.PI

            if (angle > rightBottomCornerAngle && angle <= rightTopCornerAngle) {

                // attached to right side  (region 1 in diagram)
                attachmentPoint.x = centerX + width / 2
                attachmentPoint.y = centerY + (width / 2) * tangentOfAngle
            }
            else if (angle > rightTopCornerAngle && angle <= leftTopCornerAngle) {

                // attached to top side  (region 2 in diagram)
                attachmentPoint.x = centerX + (height / (2 * tangentOfAngle))
                attachmentPoint.y = centerY + height / 2
            }
            else if (angle > leftBottomCornerAngle && angle <= rightBottomCornerAngle) {

                // attached to bottom side  (region 4 in diagram)
                attachmentPoint.x = centerX - (height / (2 * tangentOfAngle))
                attachmentPoint.y = centerY - height / 2
            }
            else {

                // attached to left side (region 3 in diagram)
                attachmentPoint.x = centerX - width / 2
                attachmentPoint.y = centerY - (width / 2) * tangentOfAngle
            }
        }

        return attachmentPoint
    }

    my.getPointAtDistanceByPointAndAngle = function (point, angle, distance) {

        var newPoint = {
            "x": null,
            "y": null
        }

        var x = distance * Math.cos(angle)
        var y = distance * Math.sin(angle)
        newPoint.x = point.x + x
        newPoint.y = point.y + y

        return newPoint
    }

    my.getRelativePositionInRectangleBasedOnPointsTo = function (size, positionPointsTo) {
        var relativePointedToPosition = {}

        if (positionPointsTo == 'left-bottom') {
            relativePointedToPosition.x = 0
            relativePointedToPosition.y = 0
        }
        else if (positionPointsTo == 'right-bottom') {
            relativePointedToPosition.x = size.width
            relativePointedToPosition.y = 0
        }
        else if (positionPointsTo == 'middle-bottom') {
            relativePointedToPosition.x = size.width / 2
            relativePointedToPosition.y = 0
        }
        else if (positionPointsTo == 'left-middle') {
            relativePointedToPosition.x = 0
            relativePointedToPosition.y = size.height / 2
        }
        else if (positionPointsTo == 'right-middle') {
            relativePointedToPosition.x = size.width
            relativePointedToPosition.y = size.height / 2
        }
        else if (positionPointsTo == 'middle-middle') {  // TODO: obsolete, is 'center'
            relativePointedToPosition.x = size.width / 2
            relativePointedToPosition.y = size.height / 2
        }
        else if (positionPointsTo == 'left-top') {
            relativePointedToPosition.x = 0
            relativePointedToPosition.y = size.height
        }
        else if (positionPointsTo == 'right-top') {
            relativePointedToPosition.x = size.width
            relativePointedToPosition.y = size.height
        }
        else if (positionPointsTo == 'middle-top') {
            relativePointedToPosition.x = size.width / 2
            relativePointedToPosition.y = size.height
        }
        else { // middle-middle (aka 'center')
            relativePointedToPosition.x = size.width / 2
            relativePointedToPosition.y = size.height / 2
        }

        return relativePointedToPosition
    }

    my.getCenterPositionBasedOnPointsTo = function (position, size, positionPointsTo) {

        var centerPosition = {}

        if (positionPointsTo == 'left-bottom') {
            centerPosition.x = position.x + size.width / 2
            centerPosition.y = position.y + size.height / 2
        }
        else if (positionPointsTo == 'right-bottom') {
            centerPosition.x = position.x - size.width / 2
            centerPosition.y = position.y + size.height / 2
        }
        else if (positionPointsTo == 'middle-bottom') {
            centerPosition.x = position.x
            centerPosition.y = position.y + size.height / 2
        }
        else if (positionPointsTo == 'left-middle') {
            centerPosition.x = position.x + size.width / 2
            centerPosition.y = position.y
        }
        else if (positionPointsTo == 'right-middle') {
            centerPosition.x = position.x - size.width / 2
            centerPosition.y = position.y
        }
        else if (positionPointsTo == 'middle-middle') {  // TODO: obsolete, is 'center'
            centerPosition.x = position.x
            centerPosition.y = position.y
        }
        else if (positionPointsTo == 'left-top') {
            centerPosition.x = position.x + size.width / 2
            centerPosition.y = position.y - size.height / 2
        }
        else if (positionPointsTo == 'right-top') {
            centerPosition.x = position.x - size.width / 2
            centerPosition.y = position.y - size.height / 2
        }
        else if (positionPointsTo == 'middle-top') {
            centerPosition.x = position.x
            centerPosition.y = position.y - size.height / 2
        }
        else { // middle-middle (aka 'center')
            centerPosition.x = position.x
            centerPosition.y = position.y
        }

        return centerPosition
    }

    my.getPointOnBezierCurve = function (percent, C1, C2, C3, C4) {
        function B1(t) { return t * t * t }
        function B2(t) { return 3 * t * t * (1 - t) }
        function B3(t) { return 3 * t * (1 - t) * (1 - t) }
        function B4(t) { return (1 - t) * (1 - t) * (1 - t) }

        var invertedPercent = 1 - percent
        var pos = {};
        pos.x = C1.x * B1(invertedPercent) + C2.x * B2(invertedPercent) + C3.x * B3(invertedPercent) + C4.x * B4(invertedPercent);
        pos.y = C1.y * B1(invertedPercent) + C2.y * B2(invertedPercent) + C3.y * B3(invertedPercent) + C4.y * B4(invertedPercent);
        return pos;
    }

    my.placeSliceConnections = function (camera) {
        var sliceContainersByIdentifier = camera.sliceContainersByIdentifier
        var rootWorldSliceContainer = camera.rootWorldSliceContainer

        var sliceConnections = camera.sliceConnections

        var loopIndex
        for (loopIndex = 0; loopIndex < sliceConnections.length; loopIndex++) {
            var sliceConnection = sliceConnections[loopIndex]

            var connectionProperties = sliceConnection.worldConnection.connectionProperties

            var fromSliceContainer = sliceContainersByIdentifier[sliceConnection.worldConnection.from]
            var toSliceContainer = sliceContainersByIdentifier[sliceConnection.worldConnection.to]

            var opacityFrom = fromSliceContainer.opacity
            var opacityTo = toSliceContainer.opacity

            // FIXME: should use a gradient or use the average?
            var opacityConnection = 1.0
            if (opacityFrom > opacityTo) {
                opacityConnection = opacityFrom
            }
            else {
                opacityConnection = opacityTo
            }

            var fromContainerProperties = fromSliceContainer.worldContainer.containerProperties
            var toContainerProperties = toSliceContainer.worldContainer.containerProperties

            // TODO: check if visible
            sliceConnection.isVisible = false

            if (fromSliceContainer.isVisible && toSliceContainer.isVisible) {

                sliceConnection.isVisible = true

                sliceConnection.opacity = opacityConnection

                var fromAbsolutePosition = {}
                fromAbsolutePosition.x = fromSliceContainer.newLayout.absolutePosition.x
                fromAbsolutePosition.y = fromSliceContainer.newLayout.absolutePosition.y

                var toAbsolutePosition = {}
                toAbsolutePosition.x = toSliceContainer.newLayout.absolutePosition.x
                toAbsolutePosition.y = toSliceContainer.newLayout.absolutePosition.y

                var fromAbsoluteSize = fromSliceContainer.newLayout.absoluteSize
                var toAbsoluteSize = toSliceContainer.newLayout.absoluteSize

                var fromCenterPosition = my.findPositionInShape(fromAbsolutePosition, fromSliceContainer.newLayout.isPositionOf, fromContainerProperties.shape, fromAbsoluteSize, 'center')
                var toCenterPosition = my.findPositionInShape(toAbsolutePosition, toSliceContainer.newLayout.isPositionOf, toContainerProperties.shape, toAbsoluteSize, 'center')

                var fromCenterX = fromCenterPosition.x
                var fromCenterY = fromCenterPosition.y

                var toCenterX = toCenterPosition.x
                var toCenterY = toCenterPosition.y

                // - considering the center of the from- and to- container, determine the angle between them.
                var deltaX = fromCenterX - toCenterX
                var deltaY = fromCenterY - toCenterY
                var angleBetweenCenterOfContainers = Math.atan2(deltaY, deltaX)

                var fromDefaultAttachementY = fromCenterY
                var toDefaultAttachementY = toCenterY

                // TODO: snap angle according to from- and to- container-settings
                // Note: since the might be limited attachmentpoints on the containers,
                //       the actual angles between the attachement-points might be quite different
                //       from the center-points of the containers (you also might want to
                //       "search" for the optimum _set_ of attachment-points)

                // TODO: you may want to snap into the corners of the reactangle, which might not correspond to the 45 degrees! ("sides-and-corners" vs. "all-8-directions")
                //       possible solution: let it point to a point on a line inside the container, which is either horizontal or vertical (depending on whether its fat or thin recatange)
                //                          when you try to connect _two_ reactangles (fat or thin ones), you find the points on these lines, that make it the shortest connection between the points.
                //                          from there you can determine the attachments points and angles to the inner-points of both rectangles
                //             even better: put an elipse inside the rectangle, which is as wide as the line (mentioned above), but has some height. This prevents "hard-snapping)

                // - take the closest allowed angle, and determine the attachement-point for both containers
                var fromAttachmentPoint = {}
                var fromAttachAngle = null
                var toAttachmentPoint = {}
                var toAttachAngle = null

                if (fromContainerProperties.connectOutgoingTo == "all-directions") {
                    fromAttachAngle = angleBetweenCenterOfContainers + Math.PI
                    if (fromAttachAngle > Math.PI) {
                        fromAttachAngle = fromAttachAngle - 2 * Math.PI
                    }
                    fromAttachmentPoint = my.getContainerAttachmentPointByAngle(fromSliceContainer, fromAttachAngle)
                }
                else if (fromContainerProperties.connectOutgoingTo == "4-directions") {
                    fromAttachAngle = angleBetweenCenterOfContainers + Math.PI
                    if (fromAttachAngle > Math.PI) {
                        fromAttachAngle = fromAttachAngle - 2 * Math.PI
                    }
                    if (fromAttachAngle > (Math.PI * (-1 / 4)) && fromAttachAngle < (Math.PI * (1 / 4))) {
                        // right-side
                        fromAttachAngle = 0
                        fromAttachmentPoint = my.getContainerAttachmentPointByAngle(fromSliceContainer, fromAttachAngle)
                    }
                    else if (fromAttachAngle > (Math.PI * (1 / 4)) && fromAttachAngle < (Math.PI * (3 / 4))) {
                        // top-side
                        fromAttachAngle = Math.PI * (1 / 2)
                        fromAttachmentPoint = my.getContainerAttachmentPointByAngle(fromSliceContainer, fromAttachAngle)
                    }
                    else if (fromAttachAngle > (Math.PI * (-3 / 4)) && fromAttachAngle < (Math.PI * (-1 / 4))) {
                        // bottom-side
                        fromAttachAngle = Math.PI * (-1 / 2)
                        fromAttachmentPoint = my.getContainerAttachmentPointByAngle(fromSliceContainer, fromAttachAngle)
                    }
                    else {
                        // left-side
                        fromAttachAngle = Math.PI
                        fromAttachmentPoint = my.getContainerAttachmentPointByAngle(fromSliceContainer, fromAttachAngle)
                    }
                }
                else {
                    // TODO: now defaulting to left-to-right, better to default to all-directions?
                    fromAttachAngle = 0
                    fromAttachmentPoint.x = fromCenterX + fromAbsoluteSize.width / 2
                    fromAttachmentPoint.y = fromDefaultAttachementY
                }

                if (toContainerProperties.connectIncomingTo == "all-directions") {
                    toAttachAngle = angleBetweenCenterOfContainers // + Math.PI
                    //if (toAttachAngle > Math.PI) {
                    //    toAttachAngle = toAttachAngle - 2 * Math.PI
                    //}
                    toAttachmentPoint = my.getContainerAttachmentPointByAngle(toSliceContainer, toAttachAngle)
                }
                else if (toContainerProperties.connectIncomingTo == "4-directions") {
                    toAttachAngle = angleBetweenCenterOfContainers
                    if (toAttachAngle > (Math.PI * (-1 / 4)) && toAttachAngle < (Math.PI * (1 / 4))) {
                        // right-side
                        toAttachAngle = 0
                        toAttachmentPoint = my.getContainerAttachmentPointByAngle(toSliceContainer, toAttachAngle)
                    }
                    else if (toAttachAngle > (Math.PI * (1 / 4)) && toAttachAngle < (Math.PI * (3 / 4))) {
                        // top-side
                        toAttachAngle = Math.PI * (1 / 2)
                        toAttachmentPoint = my.getContainerAttachmentPointByAngle(toSliceContainer, toAttachAngle)
                    }
                    else if (toAttachAngle > (Math.PI * (-3 / 4)) && toAttachAngle < (Math.PI * (-1 / 4))) {
                        // bottom-side
                        toAttachAngle = Math.PI * (-1 / 2)
                        toAttachmentPoint = my.getContainerAttachmentPointByAngle(toSliceContainer, toAttachAngle)
                    }
                    else {
                        // left-side
                        toAttachAngle = Math.PI
                        toAttachmentPoint = my.getContainerAttachmentPointByAngle(toSliceContainer, toAttachAngle)
                    }
                }
                else {
                    // TODO: now defaulting to left-to-right, better to default to all-directions?
                    toAttachAngle = Math.PI
                    toAttachmentPoint.x = toCenterX - toAbsoluteSize.width / 2
                    toAttachmentPoint.y = toDefaultAttachementY
                }

                var toLineEndPoint = toAttachmentPoint
                var showArrowHead = false

                var fromToDistanceX = Math.abs(toAttachmentPoint.x - fromAttachmentPoint.x)
                var fromToDistanceY = Math.abs(toAttachmentPoint.y - fromAttachmentPoint.y)
                var fromToDistance = Math.sqrt((fromToDistanceX * fromToDistanceX) + (fromToDistanceY * fromToDistanceY))

                var arrowPointLength = 10
                if (connectionProperties.arrowPointNormalizedLength != null) {
                    // FIXME: whould we really use the toSliceContainer this way here?
                    arrowPointLength = connectionProperties.arrowPointNormalizedLength * (toAbsoluteSize.height / toSliceContainer.worldContainer.normalizedSize.height)
                }

                var toArrowPointLeftSide = null
                var toArrowPointRightSide = null

                // TODO: use a proper criterium whether or not to draw the arrowhead
                if (toAbsoluteSize.width > arrowPointLength && toAbsoluteSize.height > arrowPointLength && fromToDistance > arrowPointLength) {  // TODO: maybe dependend on toAttachAngle?
                    showArrowHead = true

                    // - determine the arrow-startpoint: endpoint "minus" the arrow "height" (considering the angle)
                    var toArrowStartPoint = my.getPointAtDistanceByPointAndAngle(toAttachmentPoint, toAttachAngle, arrowPointLength)

                    toLineEndPoint = toArrowStartPoint

                    // - determine the arrowhead-side-points (left and right)
                    var arrowPointSideAngle = (Math.PI / 2) / 2 // 90 degrees pointy arrow
                    var arrowPointSideLength = arrowPointLength / Math.cos(arrowPointSideAngle)

                    var arrowPointLeftAngle = toAttachAngle - (arrowPointSideAngle)
                    if (arrowPointLeftAngle < -Math.PI) {
                        arrowPointLeftAngle += Math.PI * 2
                    }
                    toArrowPointLeftSide = my.getPointAtDistanceByPointAndAngle(toAttachmentPoint, arrowPointLeftAngle, arrowPointSideLength)

                    var arrowPointRightAngle = toAttachAngle + (arrowPointSideAngle)
                    if (arrowPointRightAngle > Math.PI) {
                        arrowPointRightAngle -= Math.PI * 2
                    }
                    toArrowPointRightSide = my.getPointAtDistanceByPointAndAngle(toAttachmentPoint, arrowPointRightAngle, arrowPointSideLength)
                }

                // - determine the both "bending-points": basicly a certain distance (determined by the distance between the containers) from the attachment-point (using the allowed angle)
                // TODO: using a made-up "derivative" of the real distance
                var bendingDistance = (fromToDistanceX + fromToDistanceY) / 4
                var toBendingPoint = my.getPointAtDistanceByPointAndAngle(toLineEndPoint, toAttachAngle, bendingDistance)
                var fromBendingPoint = my.getPointAtDistanceByPointAndAngle(fromAttachmentPoint, fromAttachAngle, bendingDistance)


                // Place the containers (that are children of the connection)

                sliceConnection.sliceChildren = []
                var childIndex
                for (childIndex = 0; childIndex < sliceConnection.worldConnection.worldChildren.length; childIndex++) {
                    var worldChildContainer = sliceConnection.worldConnection.worldChildren[childIndex]

                    var worldSliceChildContainer = my.createWorldSliceContainerFromWorldContainer(camera, worldChildContainer)

                    var percentageOfCurve = worldChildContainer.positionAsPercentageOfCurve
                    var childContainerPosition = my.getPointOnBezierCurve(percentageOfCurve, fromAttachmentPoint, fromBendingPoint, toBendingPoint, toLineEndPoint)

                    var childContainerWidth = worldChildContainer.normalizedSize.width * worldSliceChildContainer.newLayout.absoluteScale
                    var childContainerHeight = worldChildContainer.normalizedSize.height * worldSliceChildContainer.newLayout.absoluteScale
                    worldSliceChildContainer.newLayout.absoluteSize.width = childContainerWidth
                    worldSliceChildContainer.newLayout.absoluteSize.height = childContainerHeight

                    worldSliceChildContainer.newLayout.absolutePosition = childContainerPosition

                    // FIXME:
                    // FIXME:
                    // FIXME:
                    // FIXME:
                    worldSliceChildContainer.newLayout.isPositionOf = 'center' // FIXME HARDCODED!
                    worldSliceChildContainer.newLayout.positionOriginatesFrom = 'center' // FIXME HARDCODED!

                    // FIXME: hardcoded to true!
                    worldSliceChildContainer.isVisible = true


                    // FIXME ADD THIS AGAIN!
                    // FIXME ADD THIS AGAIN!
                    // FIXME ADD THIS AGAIN!
                    // FIXME ADD THIS AGAIN!
                    sliceConnection.sliceChildren.push(worldSliceChildContainer)
                }

                sliceConnection.isVisible = true

                sliceConnection.fromAttachmentPoint = fromAttachmentPoint
                sliceConnection.fromAttachAngle = fromAttachAngle
                sliceConnection.toAttachmentPoint = toAttachmentPoint
                sliceConnection.toAttachAngle = toAttachAngle

                sliceConnection.toLineEndPoint = toLineEndPoint

                sliceConnection.toBendingPoint = toBendingPoint
                sliceConnection.fromBendingPoint = fromBendingPoint

                sliceConnection.showArrowHead = showArrowHead
                sliceConnection.toArrowPointLeftSide = toArrowPointLeftSide
                sliceConnection.toArrowPointRightSide = toArrowPointRightSide
            }
        }
    }
    
    return my
}()