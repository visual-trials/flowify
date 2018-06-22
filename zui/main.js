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

/*

    Keyboard commands:

        s  - toggle sidebar on/off
        d  - log the currently selected container
        g  - toggle showing grid on
        m  - toggle fine mouse scrolling

        F7 - reset main camera
        F9 - enable logging (pipelined canvas)

    Special mouse commands:

        Double click in overview camera rectangle - Store world
        Double click in container-info rectangle  - Restore world

 */


ZUI.main = function () {
    "use strict"

    var my = {}

    my.worlds = {}

    my.currentTime = null
    my.showRedrawTimes = false
    my.usePipelinedContext = false

    my.fineMouseScrolling = false
    my.showGrid = false

    my.showSideBar = true
    my.showOverviewCamera = true
    my.showGraph = false
    my.showLog = true
    my.doDrawGuides = false

    my.tempStoredWorldJSON = null

    // TODO: these should be part of the camera settings
    my.maxZoomLevel = 16
    my.minZoomLevel = 1

    my.mainCamera = null
    my.overviewCamera = null

    my.containerUpdatesPerMinute = {}
    my.lastUpdateOnMinute = null

    my.mainLoop = function () {

        // STEP 1: get input from (previous) camera's and keyboard/clipboard

        // STEP 2: change the world and settings caused by input

        // STEP 3: (start) loading words from and (start) storing worlds to a backend

        // STEP 4: determine layout of screen (animated) and create/destroy camera's

        // STEP 5: draw each camera / screenpart

        for (var worldIdentifier in my.worlds) {
            if (my.worlds.hasOwnProperty(worldIdentifier)) {
                var world = my.worlds[worldIdentifier]

                if (world.mustReloadWorld) {
                    if (!world.reloadingWorld) {
                        world.reloadingWorld = true

                        world.reloadWorld(world)

                        // FIXME: this should be done by the reloadWorld-function itself (which might run asynchronously
                        world.reloadingWorld = false
                        world.mustReloadWorld = false
                    }
                }

            }
        }

        my.redrawCanvas()

        ZUI.canvas.requestAnimFrame(my.mainLoop)
    }

    my.redrawCanvas = function () {

        my.currentTime = new Date()

        ZUI.canvas.startRedrawTiming()

        ZUI.canvas.resizeCanvas()

        ZUI.canvas.clearCanvas()  // TODO: use a clearCamera function instead?

        if (ZUI.input.keyHasGoneDown && ZUI.input.keyThatIsDown === 120) {  // F9
            ZUI.canvas.pipelinedContext2d.doLogging = true
        }

		if (ZUI.input.keyHasGoneDown && ZUI.input.keyThatIsDown === 83) {  // 's'  (this is the ascii code for 'S' though...)
			my.showSideBar = !my.showSideBar
		}

        // TODO: create worlds-config (including cameras on these worlds and their dependencies!
        // FIXME: hardcoded to have only one world!

        var mainWorld = my.worlds.main

        /*
        for (var worldIdentifier in my.worlds) {
            if (my.worlds.hasOwnProperty(worldIdentifier)) {
                mainWorld = my.worlds[worldIdentifier]
            }
        }
        */

        var canvasWidthPx = ZUI.canvas.canvasElement.width
        var canvasHeightPx = ZUI.canvas.canvasElement.height

        //
        // Setting up screen division
        //

        var zuiRectanglePx = {
            pixelPosition: {
                leftPx: 0,
                topPx: 0
            },
            pixelSize: {
                widthPx: canvasWidthPx,
                heightPx: canvasHeightPx
            }
        }

        var sideBarRectanglePx = null
        var containerInfoRectanglePx = null
        var overviewCameraRectanglePx = null
        var graphRectanglePx = null

        var containerInfoCamera = {}
        var graphCamera = {}
        var logCamera = {}

        if (my.showSideBar) {

            var zuiAndSideBarRectangles = ZUI.canvas.splitRectanglePxHorizontallyUsingPixels(zuiRectanglePx, zuiRectanglePx.pixelSize.widthPx - canvasWidthPx * 0.20)

            zuiRectanglePx = zuiAndSideBarRectangles.leftRectanglePx
            sideBarRectanglePx = zuiAndSideBarRectangles.rightRectanglePx

            if (my.showOverviewCamera) {
                var containerInfoAndOverviewRectangles = ZUI.canvas.splitRectanglePxVerticallyUsingPercentage(sideBarRectanglePx, 0.60)

                containerInfoRectanglePx = containerInfoAndOverviewRectangles.topRectanglePx
                overviewCameraRectanglePx = containerInfoAndOverviewRectangles.bottomRectanglePx

                // FIXME: overviewCamera.context2d = ZUI.canvas.context2d
            }
            else {
                containerInfoRectanglePx = sideBarRectanglePx
            }

            // TODO: add width/height/left/top Px to camera
        }

        if (my.showGraph) {

            var zuiAndGraphRectangles = ZUI.canvas.splitRectanglePxVerticallyUsingPixels(zuiRectanglePx, zuiRectanglePx.pixelSize.heightPx - 200)

            zuiRectanglePx = zuiAndGraphRectangles.topRectanglePx
            graphRectanglePx = zuiAndGraphRectangles.bottomRectanglePx

            // TODO: add width/height/left/top Px to camera
        }


        // TODO: this should probably be done elsewhere (we are assuming redrawCanvas get called before any containerUpdates are reveived in the current minute!)
        var currentDate = new Date()
        var currentMinute = currentDate.getMinutes()
        if (my.lastUpdateOnMinute == null || my.lastUpdateOnMinute != currentMinute) {
            my.lastUpdateOnMinute = currentMinute

            // resetting new minute data
            my.containerUpdatesPerMinute[currentMinute] = 0
        }

        // TODO: we should create the world (based on loaded data) before calling redrawCanvas

        //
        // Foreach camera
        //

        // TODO: we should FIRST process all inputs for all cameras and THEN draw them



        {

            // Main Camera
            {
                if (ZUI.input.keyHasGoneDown) {

                    if (ZUI.input.keyThatIsDown === 118) {  // F7
                        // FIXME: note that resetting the camera will not always properly reset that container(body)s are shown again, if they weren't shown because they became too small! When you zoom out or in it WILL restore itself!

                        ZUI.camera.resetCameraPositionInCookie(my.mainCamera, 'cameraPos')
                        my.mainCamera = null
                        console.log('Resetted main camera')
                    }

                    if (ZUI.input.keyThatIsDown === 68) {  // 'd'  (this is the ascii code for 'D' though...)
                        if (my.mainCamera.selectedSliceContainer != null) {
                            console.log("Selected slice container:")
                            console.log(my.mainCamera.selectedSliceContainer)
                        }
                    }

                    if (ZUI.input.keyThatIsDown === 71) {  // 'g'  (this is the ascii code for 'G' though...)
                        my.showGrid = !my.showGrid
                    }

                    if (ZUI.input.keyThatIsDown === 77) {  // 'm'  (this is the ascii code for 'M' though...)
                        my.fineMouseScrolling = !my.fineMouseScrolling
                    }
                }

                if (my.mainCamera == null) {
                    my.mainCamera = ZUI.camera.createNewCamera(mainWorld, {x: 0, y: 0}, 1 / 1, 1 / 1)
                    if (my.usePipelinedContext) {
                        my.mainCamera.context2d = ZUI.canvas.pipelinedContext2d
                    }
                    else {
                        my.mainCamera.context2d = ZUI.canvas.context2d
                    }

                    ZUI.camera.setCameraPositionUsingCookie(my.mainCamera, 'cameraPos')
                }
                ZUI.camera.storeCameraPositionInCookie(my.mainCamera, 'cameraPos')

                my.mainCamera.pixelSize = zuiRectanglePx.pixelSize
                my.mainCamera.pixelPosition = zuiRectanglePx.pixelPosition

                ZUI.canvas.context2d.save()
                ZUI.canvas.clipCameraRectangle(my.mainCamera)

                if (my.mainCamera.hasOwnProperty('rootWorldSliceContainer')) {

                    if (mainWorld.editModeToggleable) {
                        if (ZUI.input.ctrlKeyIsDown) {
                            mainWorld.editMode = true
                        }
                        else {
                            mainWorld.editMode = false
                        }
                    }

                    ZUI.camera.processInputForCamera(my.mainCamera, ZUI.input)

                    if (my.mainCamera.selectedSliceContainer != null) {
                        // FIXME: this assumes containerData.label is always present!
                        ZUI.input.textToCopyFrom = my.mainCamera.selectedSliceContainer.worldContainer.containerData.label

                        // FIXME: temporary feature
                        if (ZUI.input.textHasComeFromClipboard) {
                            ZUI.log(ZUI.input.textComingFromClipboard)
                        }
                    }
                    else {
                        ZUI.input.textToCopyFrom = null
                    }

                }

                if (my.usePipelinedContext) {
                    ZUI.canvas.pipelinedContext2d.reset()
                }

                ZUI.camera.drawCamera(my.mainCamera)

                if (my.usePipelinedContext) {
                    ZUI.canvas.pipelinedContext2d.executeCommands()
                    ZUI.canvas.pipelinedContext2d.reset()
                }

                ZUI.canvas.context2d.restore()
            }

            if (my.showSideBar) {
 
                // SideGUI

                // TODO: should we introduce show hover vs show selected-only?
                var containerToDraw = my.mainCamera.selectedSliceContainer  // FIXME: which container do we want to show?

                // FIXME: we should first determine WHICH camera is 'in focus' and get the hovering container from that!
                if (containerToDraw == null && my.mainCamera.hoveringSliceContainer != null) {
                    containerToDraw = my.mainCamera.hoveringSliceContainer
                }

                containerInfoCamera.context2d = ZUI.canvas.context2d

                containerInfoCamera.pixelPosition = containerInfoRectanglePx.pixelPosition
                containerInfoCamera.pixelSize = containerInfoRectanglePx.pixelSize

                ZUI.render.drawContainerInfoGUIPx(containerInfoCamera, containerToDraw)

                ZUI.canvas.drawShadowNextToRectanglePx(sideBarRectanglePx.pixelPosition, sideBarRectanglePx.pixelSize, 'left', 10, 0.08)

                if (my.showOverviewCamera) {

                    // Overview camera

                    if (my.overviewCamera == null) {
                        my.overviewCamera = ZUI.camera.createNewCamera(mainWorld, {x: 0, y: 0}, 1 / 6, 1 / 1)
                        if (my.usePipelinedContext) {
                            my.overviewCamera.context2d = ZUI.canvas.pipelinedContext2d
                        }
                        else {
                            my.overviewCamera.context2d = ZUI.canvas.context2d
                        }
                    }

                    my.overviewCamera.pixelSize = overviewCameraRectanglePx.pixelSize
                    my.overviewCamera.pixelPosition = overviewCameraRectanglePx.pixelPosition

                    // FIXME: hardcoded horizontal line
                    ZUI.canvas.context2d.beginPath()
                    ZUI.canvas.context2d.lineWidth = "1"
                    ZUI.canvas.context2d.strokeStyle = "rgba(150, 150, 150, 0.3)"
                    ZUI.canvas.context2d.rect(my.overviewCamera.pixelPosition.leftPx, my.overviewCamera.pixelPosition.topPx, my.overviewCamera.pixelSize.widthPx, 0)
                    ZUI.canvas.context2d.stroke()

                    ZUI.canvas.context2d.save()
                    ZUI.canvas.clipCameraRectangle(my.overviewCamera)

                    ZUI.camera.movePositionOfCameraBasedOnMouseClickInOverviewCamera(my.mainCamera, my.overviewCamera, ZUI.input)

                    if (ZUI.input.mouseButtonHasGoneDownTwice) {
                        if (ZUI.camera.pixelPositionIsInCamera(containerInfoCamera, ZUI.input.mouseLeftPx, ZUI.input.mouseTopPx)) {
                            if (my.tempStoredWorldJSON != null) {
                                var world = ZUI.world.createWorldFromStoredWorld(my.tempStoredWorldJSON)

                                my.worlds.main = world // FIXME: this might not work if mainWorld is used after this!

                                // FIXME: we should also restore the backend! We should compare all changes and store it in the backend!

                                // FIXME: we should do this differently!
                                my.mainCamera.world = my.worlds.main
                                my.overviewCamera.world = my.worlds.main

                                console.log("Restored world")
                            }
                        }
                    }

                    if (ZUI.input.mouseButtonHasGoneDownTwice) {
                        if (ZUI.camera.pixelPositionIsInCamera(my.overviewCamera, ZUI.input.mouseLeftPx, ZUI.input.mouseTopPx)) {
                            if (my.worlds.main != null) {
                                my.tempStoredWorldJSON = ZUI.world.createStoredWorldFromWorld(my.worlds.main)

                                console.log("Stored world")
                                console.log(my.tempStoredWorldJSON)
                            }
                        }
                    }

                    // ZUI.camera.processInputForCamera(my.overviewCamera, my.overviewCamera.rootWorldSliceContainer)  // TODO: allow some movement/zooming of this camera? Just not selecting containers?
                    if (my.usePipelinedContext) {
                        ZUI.canvas.pipelinedContext2d.reset()
                    }
                    ZUI.camera.drawCamera(my.overviewCamera)
                    if (my.usePipelinedContext) {
                        ZUI.canvas.pipelinedContext2d.executeCommands()
                        ZUI.canvas.pipelinedContext2d.reset()
                    }

                    // Draw outline of main camera on overview camera
                    
                    {
                        var mainCameraPosition = {
                            "x" : my.mainCamera.centerPosition.x,
                            "y" : my.mainCamera.centerPosition.y
                        }

                        var mainCameraPositionInSecondCameraZ = ZUI.camera.getXYForDifferentZ(mainCameraPosition, my.mainCamera.zLevel, my.overviewCamera.zLevel)

                        var overviewCameraOffsetX = mainCameraPositionInSecondCameraZ.x - my.overviewCamera.centerPosition.x
                        var overviewCameraOffsetY = mainCameraPositionInSecondCameraZ.y - my.overviewCamera.centerPosition.y

                        var middleOfMainCameraOnSecondCameraLeftPx = my.overviewCamera.pixelPosition.leftPx + my.overviewCamera.pixelSize.widthPx / 2 + overviewCameraOffsetX * my.overviewCamera.pixelsPerMeter
                        var middleOfMainCameraOnSecondCameraTopPx = my.overviewCamera.pixelPosition.topPx + my.overviewCamera.pixelSize.heightPx / 2 - overviewCameraOffsetY * my.overviewCamera.pixelsPerMeter

                        // FIXME: this assumes the same pixelsPerMeter!!
                        var  mainCameraWidthOnSecondCameraPx =  my.mainCamera.pixelSize.widthPx * (my.overviewCamera.zLevel / my.mainCamera.zLevel)
                        var  mainCameraHeightOnSecondCameraPx =  my.mainCamera.pixelSize.heightPx * (my.overviewCamera.zLevel / my.mainCamera.zLevel)

                        ZUI.canvas.context2d.beginPath()
                        ZUI.canvas.context2d.rect(
                            middleOfMainCameraOnSecondCameraLeftPx - mainCameraWidthOnSecondCameraPx / 2,
                            middleOfMainCameraOnSecondCameraTopPx - mainCameraHeightOnSecondCameraPx / 2,
                            mainCameraWidthOnSecondCameraPx,
                            mainCameraHeightOnSecondCameraPx
                        );
                        ZUI.canvas.context2d.lineWidth = 1
                        ZUI.canvas.context2d.strokeStyle = "rgba(0, 0, 0, 0.7)"
                        ZUI.canvas.context2d.stroke()
                    }

                    ZUI.canvas.context2d.restore()
                }

            }

        }

        // TODO above, we are drawing on the whole canvas (and even outside it), but this is inefficient. We should only draw on the ZUI-part of the canvas

        if (my.showGraph) {
            graphCamera.context2d = ZUI.canvas.context2d
            graphCamera.pixelSize = graphRectanglePx.pixelSize
            graphCamera.pixelPosition = graphRectanglePx.pixelPosition

            // FIXME: we broke drawBarGraphPx to be used for this purpose. We might want to fix it!
            // FIXME: ZUI.render.drawBarGraphPx(graphCamera, my.containerUpdatesPerMinute)
//            ZUI.canvas.context2d.clearRect(graphLeftPx, graphTopPx, graphWidthPx, graphHeightPx)
        }

        ZUI.input.resetMouseData()
        ZUI.input.resetTouchData()
        ZUI.input.resetKeyboardData()

        if (my.showLog) {
            ZUI.canvas.drawLog();
        }

        ZUI.canvas.endRedrawTiming()

        if (my.showRedrawTimes) {
            ZUI.canvas.drawRedrawTimes()
        }

        ZUI.canvas.pipelinedContext2d.doLogging = false

    }

    return my

}()
