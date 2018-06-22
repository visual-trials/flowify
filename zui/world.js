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

ZUI.world = function () {

    var my = {}

    my.characterWidths = {}

    my.createNewWorld = function () {
        var world = {}

        world.rootContainer = null
        world.containersByIdentifier = {}
        world.connections = []

        world.containerTypeProperties = null
        world.connectionTypeProperties = null

        world.positionsOrSizesHaveChanged = false   // FIXME: do we need this for triggering the recalculation of slice-position and slice-sizes?

        world.mustReloadWorld = true
        world.reloadingWorld = false
        world.reloadWorld = null   // FIXME: this should always call a method in the API, which then sends it to an external function // This method has to be set externally
        world.initialContainerIdentifierToCenterOn = null

        // TODO: shouldnt this be a camera option?
        world.editMode = false
        world.editModeToggleable = false

        world.guidePoints = []
        world.guideLines = []

        return world
    }

    my.clearWorldContent = function (world) {
        if (world == null) {
            world = my.createNewWorld(world)
        }

        world.rootContainer = null
        world.containersByIdentifier = {}
        world.connections = []

        return world
    }

    my.stripContainerPropertiesFromContainers = function (worldContainer) {

        delete worldContainer.containerProperties

        for (var loopIndex = 0; loopIndex < worldContainer.worldChildren.length; loopIndex++) {
            var worldChildContainer = worldContainer.worldChildren[loopIndex]
            my.stripContainerPropertiesFromContainers(worldChildContainer)
        }

        return worldContainer
    }

    my.stripConnectionAndContainersPropertiesInConnections = function (worldConnections) {

        for (var connectionIndex = 0; connectionIndex < worldConnections.length; connectionIndex++) {

            var worldConnection = worldConnections[connectionIndex]

            delete worldConnection.connectionProperties

            for (var loopIndex = 0; loopIndex < worldConnection.worldChildren.length; loopIndex++) {
                var worldChildContainer = worldConnection.worldChildren[loopIndex]
                my.stripContainerPropertiesFromContainers(worldChildContainer)
            }
        }

        return worldConnections
    }

    my.createStoredWorldFromWorld = function (world) {
        var storedWorld = {}

        storedWorld.rootContainer = my.stripContainerPropertiesFromContainers(JSON.parse(JSON.stringify(world.rootContainer)))
        storedWorld.connections = my.stripConnectionAndContainersPropertiesInConnections(JSON.parse(JSON.stringify(world.connections)))
        storedWorld.containerTypeProperties = world.containerTypeProperties
        storedWorld.connectionTypeProperties = world.connectionTypeProperties

        // TODO: shouldnt this be a camera option?
        storedWorld.editMode = world.editMode
        storedWorld.editModeToggleable = world.editModeToggleable

        return JSON.stringify(storedWorld)
    }

    my.recreateAllContainersForWorld = function (worldContainer, world) {

        // TODO: check for duplicates
        // TODO: check for never-ending-recursion

        worldContainer.containerProperties = my.getContainerProperties(world, worldContainer.type, worldContainer.overrulingContainerProperties)
        world.containersByIdentifier[worldContainer.identifier] = worldContainer

        for (var loopIndex = 0; loopIndex < worldContainer.worldChildren.length; loopIndex++) {
            var worldChildContainer = worldContainer.worldChildren[loopIndex]
            my.recreateAllContainersForWorld(worldChildContainer, world)
        }
    }

    my.recreateAllConnectionsForWorld = function (worldConnections, world) {

        for (var connectionIndex = 0; connectionIndex < worldConnections.length; connectionIndex++) {

            var worldConnection = worldConnections[connectionIndex]

            worldConnection.connectionProperties = my.getConnectionProperties(world, worldConnection.type, worldConnection.overrulingConnectionProperties)

            for (var loopIndex = 0; loopIndex < worldConnection.worldChildren.length; loopIndex++) {
                var worldChildContainer = worldConnection.worldChildren[loopIndex]
                my.recreateAllContainersForWorld(worldChildContainer, world)
            }
        }
    }

    my.createWorldFromStoredWorld = function (storedWorldJSON) {

        var storedWorld = JSON.parse(storedWorldJSON)

        var world = my.createNewWorld()

        world.containerTypeProperties = storedWorld.containerTypeProperties
        world.connectionTypeProperties = storedWorld.connectionTypeProperties

        world.rootContainer = storedWorld.rootContainer
        my.recreateAllContainersForWorld(world.rootContainer, world)

        world.connections = storedWorld.connections
        my.recreateAllConnectionsForWorld(world.connections, world)

        // TODO: shouldnt this be a camera option?
        world.editMode = storedWorld.editMode
        world.editModeToggleable = storedWorld.editModeToggleable

        // Since we just "loaded" the world using the storedWorld, there is no need to reload the world at this moment
        world.mustReloadWorld = false

        // FIXME: we are not setting the reloadCallback function!

        return world
    }

    my.addWorldContainer = function (world, parentContainerOrConnection, containerIdentifier, containerType, overrulingContainerProperties) {
        var worldContainer = null

        var containerProperties = my.getContainerProperties(world, containerType, overrulingContainerProperties)

        if (containerProperties != null) {
            worldContainer = {
                identifier: containerIdentifier,
                type: containerType,
                containerProperties:  containerProperties,
                    // FIXME: HACK to be backwards-compatible! PSM1.js and PSM2.js should not access this directy (or at least use 'containerProperties' instead!)
                    // FIXME: HACK to be backwards-compatible! PSM1.js and PSM2.js should not access this directy (or at least use 'containerProperties' instead!)
                    // FIXME: HACK to be backwards-compatible! PSM1.js and PSM2.js should not access this directy (or at least use 'containerProperties' instead!)
                    containerTypeProperties:  containerProperties,
                    // FIXME: deprecated with new layout functions!
                    normalizedPosition: {
                        x: null,
                        y: null,
                        isPositionOf: null
                    },
                    // FIXME: deprecated with new layout functions!
                    normalizedSize: {
                        width: null,
                        height: null
                    },
                overrulingContainerProperties: overrulingContainerProperties, // This is needed to efficiently store the container into json (without all the default containerProperties)
                positionAsPercentageOfCurve: null,
                manualPosition: {
                    x: null,
                    y: null,
                    isPositionOf: null,
                    positionOriginatesFrom: null
                },
                manualSize: {
                    width: null,
                    height: null
                },
                containerData: {
                    label: null,
                },
                worldChildren : []
            }
        }

        if (parentContainerOrConnection == null) {
            world.rootContainer = worldContainer
        }
        else {
            parentContainerOrConnection.worldChildren.push(worldContainer)
        }

        world.containersByIdentifier[containerIdentifier] = worldContainer

        return worldContainer
    }

    my.createWorldSliceContainerFromWorldContainer = function (camera, worldContainer) {

        var sliceContainersByIdentifier = camera.sliceContainersByIdentifier
        var overrulesForSliceContainers = camera.overrulesForSliceContainers

        var worldSliceContainer = {
            worldContainer: worldContainer,
            isVisible: null,
            opacity: 1.0,
            containerTextOpacity: 1.0,
            newLayout: {
                relativeScale: 1.0,
                size: {
                    width: null,
                    height: null
                },
                position: {
                    x: null,
                    y: null
                },
                absoluteScale: null,
                absoluteSize: {
                    width: null,
                    height: null
                },
                absolutePosition: {
                    x: null,
                    y: null
                },
                isPositionOf: null,  // TODO: in worldContainer?
                positionOriginatesFrom: null  // TODO: in worldContainer?
            },
            relativePosition: {
                x: null,
                y: null
            },
            position: {
                x: null,
                y: null
            },
            size: {
                width: null,
                height: null
            },
            sliceChildren : []
        }

        var overrulesForSliceContainer = {}
        if (overrulesForSliceContainers.hasOwnProperty(worldSliceContainer.worldContainer.identifier)) {
            overrulesForSliceContainer = overrulesForSliceContainers[worldSliceContainer.worldContainer.identifier]
        }

        // FIXME: loop through all keys and overrule them!
        if (overrulesForSliceContainer.hasOwnProperty('isVisible')) {
            worldSliceContainer.isVisible = overrulesForSliceContainer.isVisible
        }

        for (var loopIndex = 0; loopIndex < worldContainer.worldChildren.length; loopIndex++) {

            var worldChildContainer = worldContainer.worldChildren[loopIndex]

            var worldSliceChildContainer = my.createWorldSliceContainerFromWorldContainer(camera, worldChildContainer)

            worldSliceContainer.sliceChildren.push(worldSliceChildContainer)
        }

        sliceContainersByIdentifier[worldContainer.identifier] = worldSliceContainer // TODO: should we use a differnt (non-world) identifier for sliceContainers?

        return worldSliceContainer
    }

    my.getContainerProperties = function (world, containerType, overrulingContainerProperties) {

        var containerProperties = null
        if (containerType) {
            if (world.containerTypeProperties.hasOwnProperty(containerType)) {
                // FIXME: we now make a deep clone using serialization/deserialization. Is it not better to use a different method? (either looping through the keys or make use of prototypes?)
                containerProperties = JSON.parse(JSON.stringify(world.containerTypeProperties[containerType]))
            }
            else {
                console.log("Unknown containerType: " + containerType)
                return containerProperties
            }
        }
        else {
            console.log("Empty containerType!")
            return containerProperties
        }

        var defaultContainerProperties = {
            drawContainer: true,
            isVisibleByDefault: true,
            drawAfterConnections: false,

            dataItemGroups: {},

            // Container: shape, color, text, icon

            showContainerBody: true,
            showContainerText: false,
            showContainerIcon: false,

            shape: "rectangle",  // Options: rectangle, ellipse

            containerColor: {r: 200, g: 200, b: 200, a: 0.15},
            containerBorderColor: {r: 150, g: 150, b: 150, a: 0.15},
            containerBorderNormalizedWidth: null,

            containerText: null,
            containerTextColor:  {r: 100, g: 100, b: 100, a: 0.7},
            containerTextFont: 'Arial',
            containerTextFontHeight: 10,
            containerTextNormalizedFontHeight: null,  // FIXME: this is the old name
            containerTextCenterHorizontally: true,
            containerTextCenterVertically: true,
            containerTextCutOffTextWhenNoMoreRoom: true,
            containerTextWrapTextWhenNoMoreRoom: true,

            containerIconColor: null,
            containerIconBorderColor: null,
            containerIconShape: null,

            // Header: color, text, icon

            showHeader: false,
            showHeaderText: false,
            showHeaderIcon: false,

            headerText: null,
            headerTextCenterHorizontally: true,
            headerTextCenterVertically: true,
            headerTextCutOffTextWhenNoMoreRoom: true,
            headerTextWrapTextWhenNoMoreRoom: true,

            headerIconColor: null,
            headerIconBorderColor: null,
            headerIconShape: null,

            hasFixedHeaderHeight: false,
            headerHeight: null,
            normalizedHeaderHeight: null,
            containerHeaderColor: {r: 200, g: 200, b: 200, a: 0.15},
            containerHeaderBorderColor: { r: 100, g: 100, b: 100, a: 0.2 },

            // Positioning, sizing, padding, aspect ratio, children-layout, connections in/out

            minWidth: 0,
            minHeight:  0,
            maxWidth: null,
            maxHeight: null,

            hasPadding: false,  // FIXME: not needed anymore!
            paddingTop: 0,
            paddingBottom: 0,
            paddingLeft: 0,
            paddingRight: 0,
            paddingBetweenChildren: 0,

            keepAspectRatio: false,
            aspectRatioAtMaxSize: null,

            // Options: left-top,       middle-top,     right-top,
            //          left-middle,    middle-middle,  right-middle,          // Note: middle-middle = center
            //          left-bottom,    middle-bottom,  right-bottom,
            positionPointsTo: "left-bottom",

            // Options: verticalTopToBottom, horizontalFromSidesToCenter, normalizedRelativePositioning
            childrenFitFunction: null,

            doSizingBasedOnScale: false,

            // Options: left-top,       middle-top,     right-top,
            //          left-middle,    middle-middle,  right-middle,          // Note: middle-middle = center
            //          left-bottom,    middle-bottom,  right-bottom,
            normalizedRelativePositionPointsTo: "left-bottom",

            // FIXME: connectOutgoingTo/connectIncomingTo should be an attribute of a CONNECTION-SIDE not (only) of a container!?!?
            // Options: left, right, header-left, header-right, all-directions, 4-directions
            connectIncomingTo: "left",
            connectOutgoingTo: "right",

            fadeOutChildrenBasedOnPercentageOfNormalSize: false,
            childrenAtFullOpacityThreshold: null,
            childrenAtZeroOpacityThreshold: null,

            // Interactivity: click/select, double-click, alerting

            // TODO: add hoverable, clickable/selectable
            isSelectable: true,

            showDetailCameraWhenDoubleClicked: false,
            showBottomCameraWhenDoubleClicked: false,

            isAlerting: false,
            alertStartTime: null,
            alertDuration: 1000   // 1000 = 1 second
        }

        for (connectionPropertyKey in defaultContainerProperties) {
            if (!containerProperties.hasOwnProperty(connectionPropertyKey)) {
                containerProperties[connectionPropertyKey] = defaultContainerProperties[connectionPropertyKey]
            }
        }

        if (overrulingContainerProperties !== null) {
            for (connectionPropertyKey in overrulingContainerProperties) {
                if (overrulingContainerProperties.hasOwnProperty(connectionPropertyKey)) {
                    containerProperties[connectionPropertyKey] = overrulingContainerProperties[connectionPropertyKey]
                }
            }
        }
        return containerProperties

    }

    my.addWorldConnection =  function (world, fromContainerIdentifier, toContainerIdentifier, connectionType, overrulingConnectionProperties) {

        var worldConnection = null

        // TODO: add an identifier for connections, so you can refer to them by identifier

        var connectionProperties = my.getConnectionProperties(world, connectionType, overrulingConnectionProperties)

        if (connectionProperties != null) {
            worldConnection = {
                // TODO: implemnt this: identifier: connectionIdentifier,
                from: fromContainerIdentifier,
                to: toContainerIdentifier,
                type: connectionType,
                opacity: 1.0,  // FIXME: put this into a worldSliceConnection!!!
                connectionProperties:  connectionProperties,
                overrulingConnectionProperties: overrulingConnectionProperties,
                connectionData: {
                    label: null,
                    errorLabel: null
                },
                worldChildren : []
            }
        }

        world.connections.push(worldConnection)

        return worldConnection
    }

    my.createWorldSliceConnectionsFromWorldConnections = function (camera, worldConnections) {

        camera.sliceConnections = []

        var loopIndex
        for (loopIndex = 0; loopIndex < worldConnections.length; loopIndex++) {
            var worldConnection = worldConnections[loopIndex]

            var sliceConnection = my.createWorldSliceConnectionFromWorldConnection (camera, worldConnection)

            camera.sliceConnections.push(sliceConnection)
        }
    }

    my.createWorldSliceConnectionFromWorldConnection = function (camera, worldConnection) {


        var worldSliceConnection = {
            worldConnection: worldConnection,
            sliceChildren : []
        }

        /*

        // TODO: implement overrulesForSliceConnection (this requires the connecitons to have an identifier)

        var sliceConnectionsByIdentifier = camera.sliceConnectionsByIdentifier
        var overrulesForSliceConnections = camera.overrulesForSliceConnections

        var overrulesForSliceConnection = {}
        if (overrulesForSliceConnections.hasOwnProperty(worldSliceConnection.worldConnection.identifier)) {
            overrulesForSliceConnection = overrulesForSliceConnections[worldSliceConnection.worldConnection.identifier]
        }

        // FIXME: loop through all keys and overrule them!
        if (overrulesForSliceConnection.hasOwnProperty('isVisible')) {
            worldSliceConnection.isVisible = overrulesForSliceConnection.isVisible
        }
        */

        for (var loopIndex = 0; loopIndex < worldConnection.worldChildren.length; loopIndex++) {

            var worldChildContainer = worldConnection.worldChildren[loopIndex]

            var worldSliceChildContainer = my.createWorldSliceContainerFromWorldContainer(camera, worldChildContainer)

            worldSliceConnection.sliceChildren.push(worldSliceChildContainer)
        }

//        sliceConnectionsByIdentifier[worldConnection.identifier] = worldSliceConnection // TODO: should we use a differnt (non-world) identifier for sliceConnections?

        return worldSliceConnection
    }

    my.getConnectionProperties = function (world, connectionType, overrulingConnectionProperties) {

        var connectionProperties = null
        if (connectionType) {
            if (world.connectionTypeProperties.hasOwnProperty(connectionType)) {
                // FIXME: we now make a deep clone using serialization/deserialization. Is it not better to use a different method? (either looping through the keys or make use of prototypes?)
                connectionProperties = JSON.parse(JSON.stringify(world.connectionTypeProperties[connectionType]))
            }
            else {
                console.log("Unknown connectionType: " + connectionType)
                return connectionProperties
            }
        }
        else {
            console.log("Empty connectionType!")
            return connectionProperties
        }

        var defaultConnectionProperties = {
            arrowLineColor: {r: 100, g: 100, b: 100, a: 0.6},
            arrowHeadColor: {r: 100, g: 100, b: 100, a: 0.6},
            lineNormalizedWidth: null,
            arrowPointNormalizedLength: null

        }

        for (connectionPropertyKey in defaultConnectionProperties) {
            if (!connectionProperties.hasOwnProperty(connectionPropertyKey)) {
                connectionProperties[connectionPropertyKey] = defaultConnectionProperties[connectionPropertyKey]
            }

        }

        if (overrulingConnectionProperties !== null) {
            for (connectionPropertyKey in overrulingConnectionProperties) {
                if (overrulingConnectionProperties.hasOwnProperty(connectionPropertyKey)) {
                    connectionProperties[connectionPropertyKey] = overrulingConnectionProperties[connectionPropertyKey]
                }
            }
        }

        return connectionProperties
    }

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

    my.doSizingPositioningAndScaling = function (world, currentSliceContainer, allowedSizeInParent) {

        var containerProperties = currentSliceContainer.worldContainer.containerProperties

        var hasPadding = containerProperties.hasPadding  // FIXME: this if not used atm!
        var paddingLeft = containerProperties.paddingLeft
        var paddingRight = containerProperties.paddingRight
        var paddingTop = containerProperties.paddingTop
        var paddingBottom = containerProperties.paddingBottom
        var paddingBetweenChildren = containerProperties.paddingBetweenChildren
        var childrenLayoutFunction = containerProperties.childrenLayoutFunction
        var doSizingBasedOnScale = containerProperties.doSizingBasedOnScale

        // FIXME: set isVisible dynamically
        currentSliceContainer.isVisible = true

        if (childrenLayoutFunction === 'verticalTopToBottom') {

            // This is a topToBottom layout function

            var largestChildWidth = 0
            var sumChildrenHeight = 0
            var oneOrMoreChildrenAdded = false

            if (allowedSizeInParent == null) {   // this means: get the MaxSize

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
            else {  // This means: shrink the container (and it's children) so it fits into the allowedSize

                {   /*  DEPRECATED!! */

                    // FIXME: should we not account for relative scale for allowedSize?
                    var totalHorizontalPadding = paddingLeft + paddingRight
                    var nrOfPaddingsBetweenChildren = 0
                    if (currentSliceContainer.sliceChildren.length > 1) {
                        nrOfPaddingsBetweenChildren = currentSliceContainer.sliceChildren.length - 1
                    }
                    var totalVerticalPadding = nrOfPaddingsBetweenChildren * paddingBetweenChildren + paddingTop + paddingBottom
                    var maxChildWidth = allowedSizeInParent.width - totalHorizontalPadding
                    var maxSumChildrenHeight = allowedSizeInParent.height - totalVerticalPadding

                    if (maxSumChildrenHeight < 0) {
                        maxSumChildrenHeight = 0

                        paddingBetweenChildren = allowedSizeInParent.height * paddingBetweenChildren / totalVerticalPadding
                        paddingTop = allowedSizeInParent.height * paddingTop / totalVerticalPadding
                        paddingBottom = allowedSizeInParent.height * paddingBottom / totalVerticalPadding
                        totalVerticalPadding = allowedSizeInParent.height
                    }

                    if (maxChildWidth < 0) {
                        maxChildWidth = 0

                        paddingLeft = allowedSizeInParent.width * paddingLeft / totalHorizontalPadding
                        paddingRight = allowedSizeInParent.width * paddingRight / totalHorizontalPadding
                        totalHorizontalPadding = allowedSizeInParent.width
                    }

                    // TODO: implement this:
                    //    var childResizeMethod = null  // Options: 'shrinkChildrenEvenly', 'shrinkChildrenThatDontFit'

                    var verticalPositionChild = 0
                    for (var loopIndex = 0; loopIndex < currentSliceContainer.sliceChildren.length; loopIndex++) {
                        var childSliceContainer = currentSliceContainer.sliceChildren[loopIndex]

                        // TODO: we are currently re-doing the maxSize-ing. This shouldn't be necessary, but since we enlarge afterwards (with a HACK) we recompute the maxSize of each child here
                        var childSize = my.doSizingPositioningAndScaling(world, childSliceContainer)

                        if (childSize != null) { // TODO: be careful, childSize has been multplied with the child's relative scale (so it's in terms of the parent)

                            if (oneOrMoreChildrenAdded) {
                                verticalPositionChild -= paddingBetweenChildren
                            }

                            var maxChildHeight = maxSumChildrenHeight - sumChildrenHeight

                            if (maxChildHeight < 0) {
                                maxChildHeight = 0
                            }

                            if (childSize.width > maxChildWidth || childSize.height > maxChildHeight) {

                                // We are going to shrink or eliminate this child! Since there is not room for it's maxSized version!

                                var allowedSizeChild = {}

                                // TODO: what to do with a child that is too wide? Should we shrink it's height accordingly (to keep it's aspect ratio)?
                                allowedSizeChild.height = maxChildHeight
                                allowedSizeChild.width = maxChildWidth   // Note that this will lead to childSize.width = maxChildWidth after calling doSizingPositioningAndScaling below

                                if (childSize.width < maxChildWidth) {
                                    allowedSizeChild.width = childSize.width
                                }

                                if (childSize.height < maxChildHeight) {
                                    allowedSizeChild.height = childSize.height
                                }

                                childSize = my.doSizingPositioningAndScaling(world, childSliceContainer, allowedSizeChild) // FIXME: should we divide allowedSizeChild by the child's relative scale here?


                            }

                            if (childSize.width > largestChildWidth) {
                                largestChildWidth = childSize.width
                            }

                            childSliceContainer.newLayout.position.x = paddingLeft
                            childSliceContainer.newLayout.position.y = -paddingTop + verticalPositionChild
                            childSliceContainer.newLayout.isPositionOf = 'left-top'
                            childSliceContainer.newLayout.positionOriginatesFrom = 'left-top'

                            // HACK to not show the container...
                            if (childSize.width === 0 || childSize.height === 0) {
                                childSliceContainer.worldContainer.containerProperties.showContainerBody = false
                            }
                            else {
                                childSliceContainer.worldContainer.containerProperties.showContainerBody = true
                            }

                            sumChildrenHeight += childSize.height
                            verticalPositionChild -= childSize.height
                            oneOrMoreChildrenAdded = true
                        }

                    }

                    // FIXME: HACK to enlarge the childs to the maxWidth of the other childs! (among other things, this should keep in mind the pointsTo etc, and re-center text/containers in the child!)
                    for (var loopIndex = 0; loopIndex < currentSliceContainer.sliceChildren.length; loopIndex++) {
                        var childSliceContainer = currentSliceContainer.sliceChildren[loopIndex]

                        childSliceContainer.newLayout.size.width = largestChildWidth
                    }

                    currentSliceContainer.newLayout.size.width = largestChildWidth + totalHorizontalPadding
                    currentSliceContainer.newLayout.size.height = sumChildrenHeight + totalVerticalPadding

                }   /*  / DEPRECATED!! */

            }

        }
        else if (childrenLayoutFunction === 'horizontalLeftToRight') {

            // This is a leftToRight layout function

            var largestChildHeight = 0
            var sumChildrenWidth = 0
            var oneOrMoreChildrenAdded = false

            if (allowedSizeInParent == null) {   // this means: get the MaxSize

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

            }
            else {  // This means: shrink the container (and it's children) so it fits into the allowedSize

                {   /*  DEPRECATED!! */

                    // FIXME: should we not account for relative scale for allowedSize?
                    var nrOfPaddingsBetweenChildren = 0
                    if (currentSliceContainer.sliceChildren.length > 1) {
                        nrOfPaddingsBetweenChildren = currentSliceContainer.sliceChildren.length - 1
                    }
                    var totalHorizontalPadding = nrOfPaddingsBetweenChildren * paddingBetweenChildren + paddingLeft + paddingRight
                    var totalVetticalPadding = paddingTop + paddingBottom
                    var maxSumChildrenWidth = allowedSizeInParent.width - totalHorizontalPadding
                    var maxChildHeight = allowedSizeInParent.height - totalVetticalPadding

                    if (maxSumChildrenWidth < 0) {
                        maxSumChildrenWidth = 0

                        paddingBetweenChildren = allowedSizeInParent.width * paddingBetweenChildren / totalHorizontalPadding
                        paddingLeft = allowedSizeInParent.width * paddingLeft / totalHorizontalPadding
                        paddingRight = allowedSizeInParent.width * paddingRight / totalHorizontalPadding
                        totalHorizontalPadding = allowedSizeInParent.width
                    }

                    if (maxChildHeight < 0) {
                        maxChildHeight = 0

                        paddingTop = allowedSizeInParent.height * paddingTop / totalVetticalPadding
                        paddingBottom = allowedSizeInParent.height * paddingBottom / totalVetticalPadding
                        totalVetticalPadding = allowedSizeInParent.height
                    }

                    var horizontalPositionChild = 0
                    for (var loopIndex = 0; loopIndex < currentSliceContainer.sliceChildren.length; loopIndex++) {
                        var childSliceContainer = currentSliceContainer.sliceChildren[loopIndex]

                        var childSize = my.doSizingPositioningAndScaling(world, childSliceContainer)

                        if (childSize != null) {

                            if (oneOrMoreChildrenAdded) {
                                horizontalPositionChild += paddingBetweenChildren
                            }

                            var maxChildWidth = maxSumChildrenWidth - sumChildrenWidth

                            if (maxChildWidth < 0) {
                                maxChildWidth = 0
                            }

                            if (childSize.width > maxChildWidth || childSize.height > maxChildHeight) {
                                // We should shrink or elimiate this child! Since there is not room for it's maxSized version!

                                // TODO: what to do with a child that is too wide? Should we shrink it's height accordingly (to keep it's aspect ratio)?
                                var allowedSizeChild = {}
                                allowedSizeChild.height = maxChildHeight
                                allowedSizeChild.width = maxChildWidth   // Note that this will lead to childSize.width = maxChildWidth after calling doSizingPositioningAndScaling below

                                if (childSize.width < maxChildWidth) {
                                    allowedSizeChild.width = childSize.width
                                }

                                if (childSize.height < maxChildHeight) {
                                    allowedSizeChild.height = childSize.height
                                }

                                childSize = my.doSizingPositioningAndScaling(world, childSliceContainer, allowedSizeChild) // FIXME: should we divide allowedSizeChild by the child's relative scale here?
                            }

                            if (childSize.height > largestChildHeight) {
                                largestChildHeight = childSize.height
                            }

                            childSliceContainer.newLayout.position.x = paddingLeft + horizontalPositionChild
                            childSliceContainer.newLayout.position.y = - paddingTop
                            childSliceContainer.newLayout.isPositionOf = 'left-top'
                            childSliceContainer.newLayout.positionOriginatesFrom = 'left-top'

                            // HACK to not show the container...
                            if (childSize.width === 0 || childSize.height === 0) {
                                childSliceContainer.worldContainer.containerProperties.showContainerBody = false
                            }
                            else {
                                childSliceContainer.worldContainer.containerProperties.showContainerBody = true
                            }

                            sumChildrenWidth += childSize.width
                            horizontalPositionChild += childSize.width
                            oneOrMoreChildrenAdded = true

                        }
                    }

                    currentSliceContainer.newLayout.size.width = sumChildrenWidth + totalHorizontalPadding
                    currentSliceContainer.newLayout.size.height = largestChildHeight + totalVetticalPadding

                }   /* / DEPRECATED!! */

            }

            return { width: currentSliceContainer.newLayout.size.width * currentSliceContainer.newLayout.relativeScale,
                height: currentSliceContainer.newLayout.size.height * currentSliceContainer.newLayout.relativeScale }
        }
        else if (childrenLayoutFunction === 'manualPositioning') {

            // This is a 'manualPosition' layout function

            if (allowedSizeInParent == null) {   // this means: get the MaxSize

                var prioritizeChildren = true

                {
                    /* @Refactor:
                            1) We first want a levelOfDetail (int) as input here, and then calculate the size of the container (and it's children)
                            2) Based on the camera zoom, we then calculate (outside this function) the corresponding levelOfDetail (float), using the levelOfDetail (int) that still fits, and the one that doesn't fit the desired allowedSize
                            3) When we get the precise levelOfDetail (float) , we interpolate the size and positions of the container and it's children (not in this function, but it might burrow some parts of this function)

                            This means below should be done (differently and) outside of the childrenLayout function. Essentially it is trying to do (2) here.
                            Instead it should map the slope of increase in size between LoD-layers to camera-level increases (this cannot be done exacte, since the aspect ratio might change!)
                            Alternatively, you could "zoom" along LoD float: 1.1 -> 1.2 -> 1.3 etc (which might be weird if you jump from 1.9 to 2.1 for example)

                            Either way: we should not be doing "Sizing" based on anything (for example on Scale), since we want to be LevelOfDetail-based/driven
                    */

                    /* @Refactor:
                            We also want to decide whether we should store a manualPosition per LoD or for all LoDs. If we do the latter,
                            we need to decide how we deal with changes in the aspect ratio (if allowed).

                            Also, we now set the width and height manually too. But shouldn't we be able to position manually without sizing the parent manually?
                            If so, should we use a percentage/normalized position? Or one that assumes some level of detail (or simply the position in the highest level of detail)?
                     */

                    var currentWidth = null
                    var currentHeight = null

                    if (currentSliceContainer.worldContainer.manualSize.width != null) {
                        currentWidth = currentSliceContainer.worldContainer.manualSize.width  // FIXME: right now the manualSize is interpreted as the maxSize. Shouldn't we make this explicit?
                    }

                    if (currentSliceContainer.worldContainer.manualSize.height != null) {
                        currentHeight = currentSliceContainer.worldContainer.manualSize.height  // FIXME: right now the manualSize is interpreted as the maxSize. Shouldn't we make this explicit?
                    }

                    currentSliceContainer.newLayout.relativeScale = 1
                    // TODO: this ENLARGES the root container based on the zLevel (~scale) of the camera. We should probably do this differently
                    var positionMultiplier = 1.0  // FIXME: this is a bit of a weird way of doing this
                    if (doSizingBasedOnScale) {
                        positionMultiplier = ZUI.main.mainCamera.zLevel
                        currentWidth = currentWidth * ZUI.main.mainCamera.zLevel
                        currentHeight = currentHeight * ZUI.main.mainCamera.zLevel
                        currentSliceContainer.newLayout.relativeScale = 1 / ZUI.main.mainCamera.zLevel  // TODO: this now keeps the absolute Scale of the root container constant. We should do this differently, probably...
                    }
                }

                for (var loopIndex = 0; loopIndex < currentSliceContainer.sliceChildren.length; loopIndex++) {
                    var childSliceContainer = currentSliceContainer.sliceChildren[loopIndex]

                    var childSize = my.doSizingPositioningAndScaling(world, childSliceContainer)

                    if (childSize != null) {

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

                        var childContainerType = childSliceContainer.worldContainer.type

                        var scaleChildren = true
                        // FIXME: HACK
                        if (prioritizeChildren) {
                            if ((childContainerType === 'functionWrapper' || childContainerType === 'primitiveFunctionWrapper') && ZUI.main.mainCamera.zLevel < 1) {
                                // childSliceContainer.isVisible = false

                                if (scaleChildren) {
                                    // We want  to scale the child down. Since there is not room for it's maxSized version!
                                    childSliceContainer.newLayout.relativeScale = ZUI.main.mainCamera.zLevel * ZUI.main.mainCamera.zLevel
                                }
                                else {


                                    var allowedSizeChild = {}

                                    allowedSizeChild.width = childSize.width * ZUI.main.mainCamera.zLevel * ZUI.main.mainCamera.zLevel
                                    allowedSizeChild.height = childSize.height * ZUI.main.mainCamera.zLevel * ZUI.main.mainCamera.zLevel


                                    // TODO: should the allowedSizeChild be in terms of the scale of the child or the parent?
                                    var childSize = my.doSizingPositioningAndScaling(world, childSliceContainer, allowedSizeChild)
                                }

                            }
                        }

                        // childSliceContainer.newLayout.position.x = paddingLeft
                        // childSliceContainer.newLayout.position.y = paddingBottom + sumChildrenHeight

                        // sumChildrenHeight += childSize.height
                        // oneOrMoreChildrenAdded = true
                    }
                }

                currentSliceContainer.newLayout.size.width = currentWidth
                currentSliceContainer.newLayout.size.height = currentHeight

            }
            else {
                // FIXME: what to do here?
            }

        }
        else if (childrenLayoutFunction === 'none') {

            // This is a one-textLine layout function

            if (allowedSizeInParent == null) {   // this means: get the MaxSize

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

            }
            else {  // This means: shrink the container (and it's children) so it fits into the allowedSize

                // FIXME: should we not account for relative scale for allowedSize?
                var maxSumChildrenWidth = allowedSizeInParent.width -  paddingLeft - paddingRight
                var maxChildHeight = allowedSizeInParent.height - paddingTop - paddingBottom

                if (maxSumChildrenWidth < 0) {
                    maxSumChildrenWidth = 0
                    paddingLeft = allowedSizeInParent.width * paddingLeft / (paddingLeft + paddingRight)
                    paddingRight = allowedSizeInParent.width * paddingRight / (paddingLeft + paddingRight)
                }

                if (maxChildHeight < 0) {
                    maxChildHeight = 0
                    paddingTop = allowedSizeInParent.height * paddingTop / (paddingTop + paddingBottom)
                    paddingBottom = allowedSizeInParent.height * paddingBottom / (paddingTop + paddingBottom)
                }

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
                if (textLineWidth <= maxSumChildrenWidth && containerTextFontHeight <= maxChildHeight) {

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
                }
                else {
                    currentSliceContainer.newLayout.size.width = paddingLeft + maxSumChildrenWidth + paddingRight
                    currentSliceContainer.newLayout.size.height = paddingTop + maxChildHeight + paddingBottom
                }


            }

            currentSliceContainer.newLayout.relativeScale = 1

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

    // TODO: make this a method of the container-prototype!
    my.getContainerHeaderHeight = function (sliceContainer) {

        var containerProperties = sliceContainer.worldContainer.containerProperties

        var headerHeight = 0
        if (containerProperties.showHeader) {
            if (containerProperties.hasFixedHeaderHeight) {
                headerHeight = containerProperties.headerHeight
                if (headerHeight > sliceContainer.size.height) {
                    headerHeight = sliceContainer.size.height
                }
            }
            else {
                headerHeight = sliceContainer.size.height * (containerProperties.normalizedHeaderHeight / sliceContainer.size.normalizedHeight)
            }
        }

        return headerHeight
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

                // TODO: what if connection to header? Use it's rectangle instead of the whole container's rectangle?

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
                // TODO: also check if we have to connect to the header or not
                if (fromContainerProperties.showHeader) {
                    var fromHeaderHeight = my.getContainerHeaderHeight(fromSliceContainer)
                    fromDefaultAttachementY = fromCenterY + (fromAbsoluteSize.height / 2 - fromHeaderHeight / 2)
                }

                var toDefaultAttachementY = toCenterY
                // TODO: also check if we have to connect to the header or not
                if (toContainerProperties.showHeader) {
                    var toHeaderHeight = my.getContainerHeaderHeight(toSliceContainer)
                    toDefaultAttachementY = toCenterY + (toAbsoluteSize.height / 2 - toHeaderHeight / 2)
                }

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
                    // TODO: now defaulting to header-left-to-right, better to default to all-directions?
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
                    // TODO: now defaulting to header-left-to-right, better to default to all-directions?
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

    my.addGuidePoint = function (world, container, deltaX, deltaY) {

        var guidePoint = {
            container: container,
            deltaX: deltaX,
            deltaY: deltaY
        }

        world.guidePoints.push(guidePoint)

        return guidePoint
    }

    my.addGuideLine = function (world, fromPoint, toPoint, lineStyle) {

        var guideLine = {
            fromPosition: fromPoint,
            toPosition: toPoint,
            lineStyle: lineStyle
        }

        world.guideLines.push(guideLine)

        return guideLine
    }

    my.clearGuides = function (world) {
        world.guideLines = []
        world.guidePoints = []
    }

    return my
}()
