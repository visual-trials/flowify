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

ZUI.world = function () {

    var my = {}

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
                manualRelativeScale: null,
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
            containerTextCenterHorizontally: true,
            containerTextCenterVertically: true,
            containerTextCutOffTextWhenNoMoreRoom: true,
            containerTextWrapTextWhenNoMoreRoom: true,

            containerIconColor: null,
            containerIconBorderColor: null,
            containerIconShape: null,

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

            // Options: left-top,       middle-top,     right-top,
            //          left-middle,    middle-middle,  right-middle,          // Note: middle-middle = center
            //          left-bottom,    middle-bottom,  right-bottom,
            positionPointsTo: "left-bottom",

            // Options: verticalTopToBottom, horizontalFromSidesToCenter, normalizedRelativePositioning
            childrenFitFunction: null,

            // FIXME: connectOutgoingTo/connectIncomingTo should be an attribute of a CONNECTION-SIDE not (only) of a container!?!?
            // Options: left, right, all-directions, 4-directions
            connectIncomingTo: "left",
            connectOutgoingTo: "right",

            // Interactivity: click/select, double-click, alerting

            // TODO: add hoverable, clickable/selectable
            isSelectable: true,

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

    return my
}()
