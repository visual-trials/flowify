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

ZUI.camera = function () {

    var my = {}

    my.createNewCamera = function (world, centerPosition, zLevel, pixelsPerMeter) {

        var camera = {}

        camera.world = world

        camera.centerPosition = centerPosition
        camera.zLevel = zLevel

        camera.pixelsPerMeter = pixelsPerMeter
        camera.pixelPosition = {}
        camera.pixelSize = {}

        camera.zoomingIsEnabled = true
        camera.horizontalPanningIsEnabled = true
        camera.verticalPanningIsEnabled = true
        camera.mouseScrollBehaviour = 'zoom' // possible values: 'zoom', 'pan-vertical', 'pan-horizontal'

        camera.fitRootContainerToCamera = false
        camera.attachRootContainerToSideOfCamera = null

        camera.overrulesForSliceContainers = {}

        camera.sliceContainerWasSelected = false
        camera.selectedSliceContainer = null
        camera.sliceContainerWasSelectedCallback = world.sliceContainerWasSelectedCallback  // FIXME: this function should not be stored in and retrieved from the api!

        camera.sliceContainerWasHoveredIn = false
        camera.sliceContainerWasHoveredOut = false
        camera.hoveringSliceContainer = null
        camera.sliceContainerHoverInCallback = world.sliceContainerHoverInCallback  // FIXME: this function should not be stored in and retrieved from the api!
        camera.sliceContainerHoverOutCallback = world.sliceContainerHoverOutCallback  // FIXME: this function should not be stored in and retrieved from the api!

        camera.sliceContainerWasMoved = false
        camera.selectedSliceContainerIsBeingMoved = false
        camera.leftOverHorizontalContainerMovement = null
        camera.leftOverVerticalContainerMovement = null
        camera.sliceContainerWasMovedCallback = world.sliceContainerWasMovedCallback  // FIXME: this function should not be stored in and retrieved from the api!

        camera.sliceContainerWasDoubleClicked = false
        camera.doubleClickedSliceContainer = null
        camera.sliceContainerWasDoubleClickedCallback = world.sliceContainerWasDoubleClickedCallback  // FIXME: this function should not be stored in and retrieved from the api!

        return camera
    }

    my.pixelPositionIsInCamera = function (camera, leftPx, topPx) {
        if ((leftPx >= camera.pixelPosition.leftPx) &&
            (leftPx <= camera.pixelPosition.leftPx + camera.pixelSize.widthPx) &&
            (topPx >= camera.pixelPosition.topPx) &&
            (topPx <= camera.pixelPosition.topPx + camera.pixelSize.heightPx)) {
            return true
        }
        else {
            return false
        }
    }

    my.changeCameraPositionUsingDeltaPx = function (camera, horizontalDeltaPx, verticalDeltaPx) {

        // Determine the new camera (center) positon in the world
        camera.centerPosition.x -= horizontalDeltaPx / camera.pixelsPerMeter
        camera.centerPosition.y += verticalDeltaPx / camera.pixelsPerMeter
    }

    my.startMovingSelectedContainer = function (camera) {
        camera.selectedSliceContainerIsBeingMoved = true
        camera.leftOverHorizontalContainerMovement = 0
        camera.leftOverVerticalContainerMovement = 0
    }

    my.stopMovingSelectedContainer = function (camera) {
        camera.selectedSliceContainerIsBeingMoved = false
        camera.leftOverHorizontalContainerMovement = null
        camera.leftOverVerticalContainerMovement = null
    }

    my.changeSelectedContainerPositionUsingDeltaPxNew = function (camera, horizontalDeltaPx, verticalDeltaPx) {

        var selectedSliceContainer = camera.selectedSliceContainer
        var selectedWorldContainer = camera.selectedSliceContainer.worldContainer

        var absoluteScale = selectedSliceContainer.newLayout.absoluteScale

        var zoomRatioContainerX = absoluteScale
        var zoomRatioContainerY = absoluteScale

        // FIXME: do that for touch too!
        camera.leftOverHorizontalContainerMovement += horizontalDeltaPx / zoomRatioContainerX
        camera.leftOverVerticalContainerMovement -= verticalDeltaPx / zoomRatioContainerY

        var exactPositionX = selectedWorldContainer.manualPosition.x + camera.leftOverHorizontalContainerMovement
        var exactPositionY = selectedWorldContainer.manualPosition.y + camera.leftOverVerticalContainerMovement

        // FIXME: make this a parameter!
        var horizontalGridSize = 10
        var verticalGridSize = 10

        // FIXME: only use grid when container is 'locked' to grid... (attribute of container?)
        var containerIsOnGrid = true
        if (containerIsOnGrid) {
            // FIXME: do this for touch too!
            selectedWorldContainer.manualPosition.x = Math.round(exactPositionX / horizontalGridSize) * horizontalGridSize
            selectedWorldContainer.manualPosition.y = Math.round(exactPositionY / verticalGridSize) * verticalGridSize
        }
        else {
            selectedWorldContainer.manualPosition.x = exactPositionX
            selectedWorldContainer.manualPosition.y = exactPositionY
        }

        camera.leftOverHorizontalContainerMovement = exactPositionX - selectedWorldContainer.manualPosition.x
        camera.leftOverVerticalContainerMovement = exactPositionY - selectedWorldContainer.manualPosition.y

    }

    my.getXYForDifferentZ = function (fromPosition, fromZLevel, toZLevel) {
        var toPosition = {}

        var relativeZoomChange = toZLevel / fromZLevel

        toPosition.x = fromPosition.x * relativeZoomChange
        toPosition.y = fromPosition.y * relativeZoomChange

        return toPosition
    }

    my.changeCameraZoomUsingZoomPointPx = function (camera, zoomPointOnScreenLeftPx, zoomPointOnScreenTopPx, relativeZoomChange) {

        // Determine the zoom position from the center of the camera (pixels)
        // We want to keep the zoom point on this PIXEL position!
        var zoomPositionLeftFromCenterCameraPx = zoomPointOnScreenLeftPx - (camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2)
        var zoomPositionTopFromCenterCameraPx = zoomPointOnScreenTopPx - (camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2)

        var distanceBetweenZoomPositionAndCameraCenterPositionX = zoomPositionLeftFromCenterCameraPx / camera.pixelsPerMeter
        var distanceBetweenZoomPositionAndCameraCenterPositionY = -zoomPositionTopFromCenterCameraPx / camera.pixelsPerMeter

        // Determine the zoom position in the world (meters)
        var zoomPositionX = camera.centerPosition.x + distanceBetweenZoomPositionAndCameraCenterPositionX
        var zoomPositionY = camera.centerPosition.y + distanceBetweenZoomPositionAndCameraCenterPositionY

        // TODO: this logic is now sort of "duplicated" (in updateCanvas). Should be using the function.
        // Change the zoom
        camera.zLevel = camera.zLevel * relativeZoomChange
        // Since we can expect the slice of the world to change in size (due to the zLevel)
        // we should move the zoomPosition to the position (in meters) so that the same elements
        // stay on this zoomPosition (but zoom in or out)
        // In other words: the zoomPosition has changed in proportion to the change in zLevel
        var newZoomPositionX = zoomPositionX * relativeZoomChange
        var newZoomPositionY = zoomPositionY * relativeZoomChange

        // Determine the new camera position (in the world) bases on the new ZoomPosition and the distinance between it and the camera
        camera.centerPosition.x = newZoomPositionX - distanceBetweenZoomPositionAndCameraCenterPositionX
        camera.centerPosition.y = newZoomPositionY - distanceBetweenZoomPositionAndCameraCenterPositionY
    }

    my.positionPxIsInsideSliceContainerNew = function (camera, leftPx, topPx, sliceContainer) {

        // TODO: the hitbox of an ellipse is different than that of a rectangle!

        // Determine the position in the world (meters)
        var positionX = camera.centerPosition.x + (leftPx - (camera.pixelPosition.leftPx + camera.pixelSize.widthPx / 2)) / camera.pixelsPerMeter
        var positionY = camera.centerPosition.y - (topPx - (camera.pixelPosition.topPx + camera.pixelSize.heightPx / 2)) / camera.pixelsPerMeter

        var containerProperties = sliceContainer.worldContainer.containerProperties

        // var positionPointsTo = containerProperties.positionPointsTo

        var width = sliceContainer.newLayout.absoluteSize.width
        var height = sliceContainer.newLayout.absoluteSize.height

        var centerPosition = ZUI.layout.findPositionInShape(sliceContainer.newLayout.absolutePosition, sliceContainer.newLayout.isPositionOf, containerProperties.shape, sliceContainer.newLayout.absoluteSize, 'center')

        if (positionX > centerPosition.x - width / 2 &&
            positionX < centerPosition.x + width / 2 &&
            positionY > centerPosition.y - height / 2 &&
            positionY < centerPosition.y + height / 2) {

            return true
        }
        else {
            return false
        }
    }

    my.findSliceContainerAtPositionPx = function (camera, leftPx, topPx, currentSliceContainer) {

        var sliceContainerAtPosition = null

        // TODO: check if containers are even visible

        // If no currentSliceContainer is given, we are at the beginning of the search: we first look at all the connections
        if (currentSliceContainer == null) {

            var sliceConnections = camera.sliceConnections

            var loopIndex
            for (loopIndex = 0; loopIndex < sliceConnections.length; loopIndex++) {
                var sliceConnection = sliceConnections[loopIndex]
                if (sliceConnection.hasOwnProperty('sliceChildren')) {
                    var sliceChildren = sliceConnection.sliceChildren
                    for (var childIndex = 0; childIndex < sliceChildren.length; childIndex++) {
                        var childSliceContainer = sliceChildren[childIndex]

                        sliceContainerAtPosition = my.findSliceContainerAtPositionPx(camera, leftPx, topPx, childSliceContainer)

                        if (sliceContainerAtPosition != null) {
                            break
                        }
                    }
                }

                if (sliceContainerAtPosition != null) {
                    break
                }

            }

            // If nothing found in the connections, then start looking at the rootWorldSliceContainer
            if (sliceContainerAtPosition == null) {
                sliceContainerAtPosition = my.findSliceContainerAtPositionPx(camera, leftPx, topPx, camera.rootWorldSliceContainer)
            }

        }
        else {
            // We are looking inside the sliceChildren first
            if (currentSliceContainer.hasOwnProperty('sliceChildren')) {
                var sliceChildren = currentSliceContainer.sliceChildren
                for (var childIndex = 0; childIndex < sliceChildren.length; childIndex++) {
                    var childSliceContainer = sliceChildren[childIndex]
                    sliceContainerAtPosition = my.findSliceContainerAtPositionPx(camera, leftPx, topPx, childSliceContainer)

                    if (sliceContainerAtPosition != null) {
                        break
                    }
                }
            }

            // Lastly, we are looking at the currentSliceContainer
            if (sliceContainerAtPosition == null) {
                var containerProperties = currentSliceContainer.worldContainer.containerProperties

                // TODO: add more checks on visibility or selectability?
                if (containerProperties.drawContainer && containerProperties.isSelectable && currentSliceContainer.isVisible) {
                    if (my.positionPxIsInsideSliceContainerNew(camera, leftPx, topPx, currentSliceContainer)) {
                        sliceContainerAtPosition = currentSliceContainer
                    }
                }
            }
        }

        return sliceContainerAtPosition
    }

    my.centerCameraOnSliceContainer = function (camera, sliceContainer) {
        var positionPointsTo = sliceContainer.worldContainer.containerProperties.positionPointsTo

        var centerPosition = ZUI.layout.findPositionInShape(sliceContainer.newLayout.absolutePosition, sliceContainer.newLayout.isPositionOf, sliceContainer.worldContainer.containerProperties.shape, sliceContainer.newLayout.absoluteSize, 'center')

        camera.centerPosition.x = centerPosition.x
        camera.centerPosition.y = centerPosition.y
    }


    my.processInputForCamera = function (camera, input) {

        // IDEA: say that the mouse pointer or finger is "attached" to something: (or two attach-points)
        //
        //         - if mouse down, then try to attach (register "id" of attached thing (at a certain point!) and record starting Px-point on screen/camera)
        //         - if mouse up, unattach
        //         - if mouse out, unattach
        //         - if mouse move (and attached) move the attached thing
        //
        //         - if touch start, then try to attach (register "id" of attached thing and  record starting Px-point on screen/camera)
        //         - if touch end, unattach (if no more touches)
        //         - if touch out, unattach
        //         - if touch move (and attached) move the attached thing
        //
        //         - if two touches (and not attached?) then assume zooming start: attach two points!!
        //         - if touch move (and two attaches) then resize thing or zoom

        var world = camera.world

        // Resetting all camera-events that can occur
        camera.sliceContainerWasSelected = false
        camera.sliceContainerWasDoubleClicked = false
        camera.sliceContainerWasMoved = false
        camera.sliceContainerWasHoveredIn = false
        camera.sliceContainerWasHoveredOut = false

        // Mouse scroll wheel
        if (input.mouseWheelHasChanged) {
            if (ZUI.camera.pixelPositionIsInCamera(camera, input.mouseLeftPx, input.mouseTopPx)) {

                if (camera.mouseScrollBehaviour === 'zoom') {
                    if (camera.zoomingIsEnabled) {
                        var scrollSensitivity = 0.2
                        if (ZUI.main.fineMouseScrolling) {
                            scrollSensitivity = 0.01
                        }
                        var relativeZoomChange = 1 + Math.abs(input.mouseWheelDelta) * scrollSensitivity

                        if (input.mouseWheelDelta < 0) {
                            relativeZoomChange = 1 / relativeZoomChange
                        }

                        ZUI.camera.changeCameraZoomUsingZoomPointPx(camera, input.mouseLeftPx, input.mouseTopPx, relativeZoomChange)
                    }
                }
                else if (camera.mouseScrollBehaviour === 'pan-vertical') {
                    if (camera.verticalPanningIsEnabled) {
                        var verticalDeltaPx = input.mouseWheelDelta * 100
                        ZUI.camera.changeCameraPositionUsingDeltaPx(camera, 0, verticalDeltaPx)
                    }
                }
                else if (camera.mouseScrollBehaviour === 'pan-horizontal') {
                    if (camera.horizontalPanningIsEnabled) {
                        var horizontalDeltaPx = input.mouseWheelDelta * 100
                        ZUI.camera.changeCameraPositionUsingDeltaPx(camera, horizontalDeltaPx, 0)
                    }
                }
                else {
                    console.log('Invalid mouseScrollBehaviour: ' + camera.mouseScrollBehaviour)
                }

            }
        }

        // Mouse button down
        if (input.mouseButtonHasGoneDown) {
            if (ZUI.camera.pixelPositionIsInCamera(camera, input.mouseLeftPx, input.mouseTopPx)) {

                camera.selectedSliceContainer = ZUI.camera.findSliceContainerAtPositionPx(camera, input.mouseLeftPx, input.mouseTopPx)

                // TODO: instead of using world.editMode, determine whether we are in moving-mode and/or the selectedSliceContainer is moveable (and/or maybe the CAMERA is in editMode?)
                if (world.editMode) {
                    my.startMovingSelectedContainer(camera)
                }

                if (camera.selectedSliceContainer != null) {
                    if (!camera.sliceContainerWasSelected) {
                        camera.sliceContainerWasSelected = true
                    }
                }
                // FIXME: this should be moved to main.js! (or be run at the end of this function?)
                if (camera.sliceContainerWasSelected && camera.sliceContainerWasSelectedCallback != null) {
                    camera.sliceContainerWasSelectedCallback(camera.selectedSliceContainer)
                }

                if (input.mouseButtonHasGoneDownTwice && camera.selectedSliceContainer != null) {

                    // TODO: we should me it a parameter whether or not the camera should be centered on the doubleclicked slicecontainer
                    my.centerCameraOnSliceContainer(camera, camera.selectedSliceContainer)

                    camera.sliceContainerWasDoubleClicked = true
                    camera.doubleClickedSliceContainer = camera.selectedSliceContainer

                    // FIXME: this should be moved to main.js! (or be run at the end of this function?)
                    if (camera.sliceContainerWasDoubleClicked && camera.sliceContainerWasDoubleClickedCallback != null) {
                        camera.sliceContainerWasDoubleClickedCallback(camera.doubleClickedSliceContainer)
                    }
                }

            }
        }

        // Mouse button up
        if (input.mouseButtonHasGoneUp) {

            if (camera.selectedSliceContainer != null && camera.selectedSliceContainerIsBeingMoved) {
                if (!camera.sliceContainerWasMoved) {
                    camera.sliceContainerWasMoved = true
                }
            }
            // FIXME: this should be moved to main.js! (or be run at the end of this function?)
            if (camera.sliceContainerWasMoved && camera.sliceContainerWasMovedCallback != null) {
                camera.sliceContainerWasMovedCallback(camera.selectedSliceContainer)
            }

            my.stopMovingSelectedContainer(camera)
        }

        // Mouse movement
        if (input.mouseHasMoved) {

            // FIXME: this should be done using an 'isActiveCamera'
            // FIXME: because when you move your mouse out of the camera, you still want it to be active (and not the other camera) WHEN you have the mouse still down!

            if (ZUI.camera.pixelPositionIsInCamera(camera, input.mouseLeftPx, input.mouseTopPx)) {

                if (input.mouseLeftButtonIsDown) {

                    var horizontalDeltaPx = input.mouseLeftPx - input.lastMouseCoords[0]
                    var verticalDeltaPx = input.mouseTopPx - input.lastMouseCoords[1]

                    if (camera.selectedSliceContainer == null || !camera.selectedSliceContainerIsBeingMoved) {
                        if (!camera.horizontalPanningIsEnabled) {
                            horizontalDeltaPx = 0
                        }
                        if (!camera.verticalPanningIsEnabled) {
                            verticalDeltaPx = 0
                        }
                        ZUI.camera.changeCameraPositionUsingDeltaPx(camera, horizontalDeltaPx, verticalDeltaPx)
                    }
                    else {
                        ZUI.camera.changeSelectedContainerPositionUsingDeltaPxNew(camera, horizontalDeltaPx, verticalDeltaPx)

                        world.positionsOrSizesHaveChanged = true
                    }
                }

            }
        }

        // Mouse hovering
        if (ZUI.camera.pixelPositionIsInCamera(camera, input.mouseLeftPx, input.mouseTopPx)) {
            // FIXME: we might not want to use HoverIn and HoverOut, but instead use Hover: this is called each frame a container is hovered, and when hovered the container sets its state so that the action of being hovered only persists one frame
            var hoveringSliceContainer = ZUI.camera.findSliceContainerAtPositionPx(camera, input.mouseLeftPx, input.mouseTopPx)

            // FIXME: we probaly shouldnt store the sliceContainer, since it is very volatile (and might cause a memory-leak). Instead store the identifier of the container!

            if (camera.hoveringSliceContainer != null && (hoveringSliceContainer == null || hoveringSliceContainer.worldContainer.identifier != camera.hoveringSliceContainer.worldContainer.identifier)) {
                camera.sliceContainerWasHoveredOut = true
            }
            // FIXME: this should be moved to main.js! (or be run at the end of this function?)
            if (camera.sliceContainerWasHoveredOut && camera.sliceContainerHoverOutCallback != null) {
                camera.sliceContainerHoverOutCallback(camera, camera.hoveringSliceContainer)
            }

            if (hoveringSliceContainer != null && (camera.hoveringSliceContainer == null || hoveringSliceContainer.worldContainer.identifier != camera.hoveringSliceContainer.worldContainer.identifier)) {
                camera.sliceContainerWasHoveredIn = true
            }
            // FIXME: this should be moved to main.js! (or be run at the end of this function?)
            if (camera.sliceContainerWasHoveredIn && camera.sliceContainerHoverInCallback != null) {
                camera.sliceContainerHoverInCallback(camera, hoveringSliceContainer)
            }

            camera.hoveringSliceContainer = hoveringSliceContainer
        }

        // Touch start
        if (input.touchHasStarted) {
            if (input.oneTouchActive) {
                if (ZUI.camera.pixelPositionIsInCamera(camera, input.firstTouchLeftPx, input.firstTouchTopPx)) {

                    camera.selectedSliceContainer = ZUI.camera.findSliceContainerAtPositionPx(camera, input.firstTouchLeftPx, input.firstTouchTopPx)

                    // TODO: instead of using world.editMode, determine whether we are in moving-mode and/or the selectedSliceContainer is moveable (and/or maybe the CAMERA is in editMode?)
                    if (world.editMode) {
                        my.startMovingSelectedContainer(camera)
                    }

                    if (camera.selectedSliceContainer != null) {
                        if (!camera.sliceContainerWasSelected) {
                            camera.sliceContainerWasSelected = true
                        }
                    }
                    // FIXME: this should be moved to main.js! (or be run at the end of this function?)
                    if (camera.sliceContainerWasSelected && camera.sliceContainerWasSelectedCallback != null) {
                        camera.sliceContainerWasSelectedCallback(camera.selectedSliceContainer)
                    }

                }
            }
            else if (input.twoTouchesActive) {
                var x1 = input.firstTouchLeftPx
                var y1 = input.firstTouchTopPx
                var x2 = input.secondTouchLeftPx
                var y2 = input.secondTouchTopPx

                input.previousTouchDistance = Math.sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2))
            }
            else {
                // TODO
            }

        }

        // Touch end
        if (input.touchHasEnded) {
            // TODO: is this correct?
            if (!input.oneTouchActive && !input.twoTouchesActive) {

                if (camera.selectedSliceContainer != null && camera.selectedSliceContainerIsBeingMoved) {
                    if (!camera.sliceContainerWasMoved) {
                        camera.sliceContainerWasMoved = true
                    }
                }
                // FIXME: this should be moved to main.js! (or be run at the end of this function?)
                if (camera.sliceContainerWasMoved && camera.sliceContainerWasMovedCallback != null) {
                    camera.sliceContainerWasMovedCallback(camera.selectedSliceContainer)
                }

                my.stopMovingSelectedContainer(camera)

            }
        }

        // Touch movement
        if (input.touchHasMoved) {

            if (input.oneTouchActive) {
                var horizontalDeltaPx = input.firstTouchLeftPx - input.previousTouchCoords[0]
                var verticalDeltaPx = input.firstTouchTopPx - input.previousTouchCoords[1]

                if (camera.selectedSliceContainer == null || !camera.selectedSliceContainerIsBeingMoved) {
                    if (!camera.horizontalPanningIsEnabled) {
                        horizontalDeltaPx = 0
                    }
                    if (!camera.verticalPanningIsEnabled) {
                        verticalDeltaPx = 0
                    }
                    ZUI.camera.changeCameraPositionUsingDeltaPx(camera, horizontalDeltaPx, verticalDeltaPx)
                }
                else {
                    ZUI.camera.changeSelectedContainerPositionUsingDeltaPxNew(camera, horizontalDeltaPx, verticalDeltaPx)

                    world.positionsOrSizesHaveChanged = true
                }

            }
            else if (input.twoTouchesActive) {
                var x1 = input.firstTouchLeftPx
                var y1 = input.firstTouchTopPx
                var x2 = input.secondTouchLeftPx
                var y2 = input.secondTouchTopPx

                var touchDistance = Math.sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2))

                if (camera.zoomingIsEnabled) {
                    // FIXME: shouldnt this use camera-pixelsize? (height+width) / 2
                    var averageBorderSize = (window.innerHeight + window.innerWidth) / 2
                    var relativeZoomChange = 1 + ((touchDistance - input.previousTouchDistance) * 4) / averageBorderSize
                    var zoomPointOnScreenLeftPx = (x1 + x2) / 2
                    var zoomPointOnScreenTopPx = (y1 + y2) / 2

                    ZUI.camera.changeCameraZoomUsingZoomPointPx(camera, zoomPointOnScreenLeftPx, zoomPointOnScreenTopPx, relativeZoomChange)
                }

                input.previousTouchDistance = touchDistance

            }
            else {
                // TODO
            }

        }

    }

    my.movePositionOfCameraBasedOnMouseClickInOverviewCamera = function (camera, overviewCamera, input) {
        if (overviewCamera.hasOwnProperty('rootWorldSliceContainer')) {
            if (input.mouseLeftButtonIsDown) {
                if (ZUI.camera.pixelPositionIsInCamera(overviewCamera, input.mouseLeftPx, input.mouseTopPx)) {
                    var positionX = overviewCamera.centerPosition.x + (input.mouseLeftPx - (overviewCamera.pixelPosition.leftPx + overviewCamera.pixelSize.widthPx / 2)) / overviewCamera.pixelsPerMeter
                    var positionY = overviewCamera.centerPosition.y - (input.mouseTopPx - (overviewCamera.pixelPosition.topPx + overviewCamera.pixelSize.heightPx / 2)) / overviewCamera.pixelsPerMeter

                    var mousePositionInSecondCameraZ = {
                        x: positionX,
                        y: positionY
                    }

                    var mousePositionInMainCameraZ = ZUI.camera.getXYForDifferentZ(mousePositionInSecondCameraZ, overviewCamera.zLevel, camera.zLevel)

                    camera.centerPosition.x = mousePositionInMainCameraZ.x
                    camera.centerPosition.y = mousePositionInMainCameraZ.y
                }
            }
        }

    }

    my.resetCameraPositionInCookie = function (camera, cookieName) {
        // TODO: we are not using the camera here
        ZUI.input.storeCookie(cookieName, '')
    }

    my.storeCameraPositionInCookie = function (camera, cookieName) {
        var cameraCookie = {
            x: camera.centerPosition.x,
            y: camera.centerPosition.y,
            z: camera.zLevel
        }

        // FIXME: only store if it's different! (does it take more time to read from the cookie than to always write?)
        // FIXME: only check to store every x (milli)seconds? (where x is given in as an argment)?
        ZUI.input.storeCookie(cookieName, JSON.stringify(cameraCookie))
    }

    my.setCameraPositionUsingCookie = function (camera, cookieName) {
        var cameraPosFromCookieJSON = ZUI.input.getCookie(cookieName)
        if (cameraPosFromCookieJSON !== '') {
            var cameraPosFromCookie = JSON.parse(cameraPosFromCookieJSON)

            camera.centerPosition.x = cameraPosFromCookie.x
            camera.centerPosition.y = cameraPosFromCookie.y
            camera.zLevel = cameraPosFromCookie.z
        }
    }

    my.drawCamera = function (camera) {

        //
        // Setting up camera
        //

        var world = camera.world
        var rootContainer = world.rootContainer
        var worldConnections = world.connections

        if (rootContainer == null) return  // if no world has been loaded we should not proceed

        camera.width = camera.pixelSize.widthPx / camera.pixelsPerMeter
        camera.height = camera.pixelSize.heightPx / camera.pixelsPerMeter

        //
        // Setting up slice containers needed for camera
        //

        // TODO: will this not leak memory or tax the GC? Should we only do this when the world changes structurally? (and store this in the camera)
        camera.sliceContainersByIdentifier = {}
        camera.rootWorldSliceContainer = ZUI.world.createWorldSliceContainerFromWorldContainer(camera, rootContainer)

        // TODO: should we set these here?
        camera.rootWorldSliceContainer.newLayout.isPositionOf = 'center'
        camera.rootWorldSliceContainer.newLayout.positionOriginatesFrom = 'center'

        ZUI.layout.doSizingPositioningAndScaling(world, camera.rootWorldSliceContainer)

        ZUI.layout.setAbsoluteContainerPositionsNew({x:0, y:0}, camera.zLevel, null, camera.rootWorldSliceContainer)

        camera.sliceConnections = []  // FIXME: remove this when createWorldSliceConnectionsFromWorldConnections is called!
        ZUI.world.createWorldSliceConnectionsFromWorldConnections(camera, worldConnections)
        ZUI.layout.placeSliceConnections(camera)

        // TODO: my.centerCameraOnSliceContainer(camera, sliceContainerToCenterOn)

        //
        // Do the drawing of the containers and connections

        ZUI.render.drawContainersNew(camera, camera.rootWorldSliceContainer, false)
        ZUI.render.drawConnections(camera, camera.sliceConnections)

    }

    return my
}()
