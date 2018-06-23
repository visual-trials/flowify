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

var containerTypeProperties = {
    root: {
        drawContainer: true,
        containerColor: {r: 250, g: 250, b: 250, a: 1},
        containerBorderColor: {r: 240, g: 240, b: 240, a: 1},
        connectOutgoingTo: "right",
        connectIncomingTo: "left",
        paddingTop: 20,
        paddingBottom: 20,
        paddingLeft: 20,
        paddingRight: 20,
        paddingBetweenChildren: 20,
        minWidth: 200,
        minHeight: 50,
        maxHeight: 1500,
        childrenLayoutFunction: "manualPositioning",
    },
    functionWrapper: {
        drawContainer: true,
        showContainerBody: false,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 200, a: 0},
        childrenLayoutFunction: "verticalTopToBottom",
    },
    functionHeader: {
        drawContainer: true,
        isSelectable: false,   // FIXME: workaround to make it possible to move a functionWrapper!
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 1},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 1},
        showContainerIcon: false,
        showContainerText: true,
        connectOutgoingTo: "right",
        connectIncomingTo: "left",
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        minWidth: 25,
        minHeight: 25,
        maxHeight: 25,
        childrenLayoutFunction: "none",
    },
    function: {
        drawContainer: true,
        containerColor: {r: 255, g: 255, b: 240, a: 1},
        containerBorderColor: {r: 240, g: 240, b: 220, a: .8},
        paddingTop: 20,
        paddingBottom: 20,
        paddingLeft: 20,
        paddingRight: 20,
        paddingBetweenChildren: 20,
        minWidth: 200,
        minHeight: 50,
        maxHeight: 1500,
        childrenLayoutFunction: "horizontalLeftToRight",
    },
    primitiveFunctionWrapper: {
        drawContainer: true,
        showContainerBody: false,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 200, a: 0},
        paddingBetweenChildren: 0,
        childrenLayoutFunction: "verticalTopToBottom",
    },
    primitiveFunctionHeader: {
        drawContainer: true,
        isSelectable: false,   // FIXME: workaround to make it possible to move a primitiveFunctionWrapper!
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 1},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 1},
        showContainerIcon: false,
        showContainerText: true,
        connectOutgoingTo: "right",
        connectIncomingTo: "left",
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        minWidth: 25,
        minHeight: 25,
        maxHeight: 25,
        childrenLayoutFunction: "none",
    },
    primitiveFunction: {
        drawContainer: true,
        isSelectable: false,   // FIXME: workaround to make it possible to move a primitiveFunctionWrapper!
//        showContainerBody: false,
        containerColor: {r: 255, g: 255, b: 220, a: 1},
        containerBorderColor: {r: 240, g: 240, b: 200, a: .8},
        paddingTop: 20,
        paddingBottom: 20,
        paddingLeft: 20,
        paddingRight: 20,
        paddingBetweenChildren: 20,
        minWidth: 200,
        minHeight: 50,
        maxHeight: 1500,
        childrenLayoutFunction: "horizontalLeftToRight",
        dataItemGroups: {
            basicInfo: [
                { containerKey: "identifier", displayName: "identifier" },
                { containerKey: "type", displayName: "type" },
                { containerDataKey: "visualInfoStorageIdentifier", displayName: "visualInfoStorageIdentifier" }
            ]
        }
    },
    map: {
        drawContainer: true,
        containerColor: {r: 240, g: 245, b: 255, a: 1},
        containerBorderColor: {r: 220, g: 220, b: 255, a: 1},
        connectOutgoingTo: "right",
        connectIncomingTo: "left",
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        paddingBetweenChildren: 10,
        minWidth: 50,
        minHeight: 50,
        maxHeight: 300,
        childrenLayoutFunction: "verticalTopToBottom",
    },
    keyValue: {
        drawContainer: false,
        showContainerBody: true,
        showContainerIcon: false,
        showContainerText: false,
        connectOutgoingTo: "right",
        connectIncomingTo: "left",
        paddingBetweenChildren: 10,
        minWidth: 25,
        minHeight: 25,
        maxHeight: 45,
        childrenLayoutFunction: "horizontalLeftToRight",
    },
    value: {
        drawContainer: true,
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 1},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 1},
        showContainerIcon: false,
        showContainerText: true,
        connectOutgoingTo: "right",
        connectIncomingTo: "left",
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        minWidth: 25,
        minHeight: 25,
        maxHeight: 25,
        childrenLayoutFunction: "none",
        dataItemGroups: {
            basicInfo: [
                { containerKey: "identifier", displayName: "identifier" },
                { containerKey: "type", displayName: "type" },
                { containerDataKey: "visualInfoStorageIdentifier", displayName: "visualInfoStorageIdentifier" }
            ]
        }
    },
    input: {
        drawContainer: true,
        showContainerBody: true,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 200, a: 0},
        paddingTop: 15,
        paddingBottom: 15,
        paddingLeft: 15,
        paddingRight: 15,
        paddingBetweenChildren: 15,
        childrenLayoutFunction: "verticalTopToBottom",
    },
    output: {
        drawContainer: true,
        showContainerBody: true,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 230, a: 0},
        paddingTop: 15,
        paddingBottom: 15,
        paddingLeft: 15,
        paddingRight: 15,
        paddingBetweenChildren: 15,
        childrenLayoutFunction: "verticalTopToBottom",
    },
    body: {
        drawContainer: true,
        isSelectable: false,   // FIXME: workaround to make it possible to move a functionWrapper!
        showContainerBody: true,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 230, a: 0},
        paddingTop: 15,
        paddingBottom: 15,
        paddingLeft: 15,
        paddingRight: 15,
        paddingBetweenChildren: 30,
        childrenLayoutFunction: "manualPositioning",
    }
}
var connectionTypeProperties = {
    dataFlow: {
    }
}

var reloadWorld = function (world) {

    var url = "exampleProgramMapFunction.php"

    var xmlhttp = new XMLHttpRequest()
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            var flowData = JSON.parse(xmlhttp.responseText)

            convertFlowDataToZUIContainers(world, flowData)
        }
    }
    xmlhttp.open("GET", url, true)
    xmlhttp.send()
}

function convertFlowDataToZUIContainers (world, flowData) {


    // console.log(flowData)

    var mapFlowTypesToZUITypes = {
        RootContainer: "root",
        FunctionContainer: "function",
        PrimitiveFunctionContainer: "primitiveFunction",
        InputContainer: "input",
        BodyContainer: "function",   // FIXME: we need input, body and output containers!
        OutputContainer: "output",
        KeyValue: "keyValue",
        Int: "value",  // FIXME
        Map: "map",
        List: "map",  // FIXME

        // FIXME:add more

    }

    // TODO: a connection should have a type, most notably:

    // 1) Identity (this is the same element)
    // 2) Reference (this is a reference to the element)
    // 3) Copy (this is copy of the element)
    // 4) Conversion (this is the element, changed by some operation)

    var addContainersToWorld = function (world, parentWorldContainer, flowContainer) {

        var containerType = mapFlowTypesToZUITypes[flowContainer.type]  // FIXME: check if it exists!

        var overrulingContainerProperties = {}
        var containerText = null

        var addHeader = false
        var overrulingHeaderProperties = {}
        var headerText = null

        if (flowContainer.type === 'Int') {
            containerText = flowContainer.bareValue
        }
        else if (flowContainer.type === 'Map') {
            containerText = 'Map'  // FIXME
        }
        else if (flowContainer.type === 'List') {
            containerText = 'List'  // FIXME
        }
        else if (flowContainer.type === 'PrimitiveFunctionContainer') {
            addHeader = true
            headerText = flowContainer.name
        }
        else if (flowContainer.type === 'FunctionContainer') {
            addHeader = true
            headerText = flowContainer.name
        }
        else if (flowContainer.type === 'RootContainer') {
            containerText = 'root'
        }

        var extendWorldContainerWithVisualInfo = function (worldContainer, flowContainer) {
            if (flowContainer.hasOwnProperty("x")) {
                worldContainer.manualPosition.x = flowContainer.x
            }
            if (flowContainer.hasOwnProperty("y")) {
                worldContainer.manualPosition.y = flowContainer.y
            }
            if (flowContainer.hasOwnProperty("isPositionOf")) {
                worldContainer.manualPosition.isPositionOf = flowContainer.isPositionOf
            }
            else {
                worldContainer.manualPosition.isPositionOf = "center"  // FIXME: should this not simply be stored? (and not rely on the default of the engine! or some hardcoded value here!)
            }
            if (flowContainer.hasOwnProperty("width")) {
                worldContainer.manualSize.width = flowContainer.width
            }
            if (flowContainer.hasOwnProperty("height")) {
                worldContainer.manualSize.height = flowContainer.height
            }

            worldContainer.containerData.visualInfoStorageIdentifier = flowContainer.visualInfoStorageIdentifier
        }

        // FIXME: we should set the 'layoutFunction' for each of the containerTypes!!
        // FIXME: we should set the 'layoutFunction' for each of the containerTypes!!
        // FIXME: we should set the 'layoutFunction' for each of the containerTypes!!

        if (addHeader) {
            var worldContainerWrapper = ZUI.addWorldContainer(world, parentWorldContainer, flowContainer.id + '_Wrapper', containerType + 'Wrapper')

            // FIXME: we are adding position AND size here, but we only need to add one of them!
            extendWorldContainerWithVisualInfo(worldContainerWrapper, flowContainer)

            parentWorldContainer = worldContainerWrapper

            // FIXME: we are accessing 'containerTypeProperties' here. We should do this here AND let world.js do the same. We should pass it through the addWorldContainer-function and don't set it through the setContainerTypeProperties()
            var containerProperties = containerTypeProperties[containerType]
            overrulingHeaderProperties.containerColor = containerProperties.containerColor
            overrulingHeaderProperties.containerBorderColor = containerProperties.containerBorderColor

            overrulingHeaderProperties.containerText = headerText
            var worldContainerHeader = ZUI.addWorldContainer(world, parentWorldContainer, flowContainer.id + '_Header', containerType + 'Header', overrulingHeaderProperties)

            overrulingContainerProperties.containerColor = {r: 250, g: 250, b: 250, a: 1}
            overrulingContainerProperties.containerBorderColor = {r: 150, g: 150, b: 150, a: 0.1}
            overrulingContainerProperties.containerText = '' // FIXME: simply don't draw the text?
            // overrulingContainerProperties.containerText = containerText
            var worldContainer = ZUI.addWorldContainer(world, parentWorldContainer, flowContainer.id, containerType, overrulingContainerProperties)

            // FIXME: we are adding position AND size here, but we only need to add one of them!
            extendWorldContainerWithVisualInfo(worldContainer, flowContainer)

        }
        else {
            overrulingContainerProperties.containerText = containerText
            var worldContainer = ZUI.addWorldContainer(world, parentWorldContainer, flowContainer.id, containerType, overrulingContainerProperties)

            extendWorldContainerWithVisualInfo(worldContainer, flowContainer)

            // HACK: changing afterwards! (and only for RootContainer)
            if (flowContainer.type === 'RootContainer') {
                // FIXME: HACK!? Or can we do this in combination with the manualPositions of its children?
                worldContainer.manualSize = {}
                worldContainer.manualSize.width = 1500
                worldContainer.manualSize.height = 800
            }

        }

        if (flowContainer.hasOwnProperty('children')) {
            for (var loopIndex = 0; loopIndex < flowContainer.children.length; loopIndex++) {
                var childContainer = flowContainer.children[loopIndex]
                addContainersToWorld(world, worldContainer, childContainer)
            }
        }
    }

    ZUI.clearWorldContent(world)

    var rootContainer = flowData.containers
    var connections = flowData.connections

    addContainersToWorld(world, null, rootContainer)

    for (loopIndex = 0; loopIndex < connections.length; loopIndex++) {
        var connection = connections[loopIndex]

        // TODO: connection.type?
        ZUI.addWorldConnection(world, connection.from, connection.to, 'dataFlow')
    }

}


var sliceContainerWasMoved = function (movedSliceContainer) {
    // console.log("Container was moved:")
    // console.log(movedSliceContainer)
}

var sliceContainerWasSelected = function (selectedSliceContainer) {
    // console.log(selectedSliceContainer)
}

// TODO: guides for debugging
ZUI.main.doDrawGuides = true

var flowWorld = ZUI.world.createNewWorld()
ZUI.setContainerTypeProperties(flowWorld, containerTypeProperties)
ZUI.setConnectionTypeProperties(flowWorld, connectionTypeProperties)
ZUI.setReloadWorldFunction(flowWorld, reloadWorld)
// ZUI.setContainerWasDoubleClickedOutputListener(flowWorld, sliceContainerWasDoubleClicked)
 ZUI.setContainerHasMovedOutputListener(flowWorld, sliceContainerWasMoved)
ZUI.setContainerWasSelectedOutputListener(flowWorld, sliceContainerWasSelected)
ZUI.setEditModeToggleable(flowWorld, true)

// ZUI.setContainerTypeProperties(containerTypeProperties)
ZUI.setReloadWorldFunction(flowWorld, reloadWorld)
ZUI.setContainerHasMovedOutputListener(sliceContainerWasMoved)
ZUI.setContainerWasSelectedOutputListener(sliceContainerWasSelected)
ZUI.configureInterface(
    {
        worlds: {
            main: flowWorld
        }
    }
)
ZUI.addInputListeners()
ZUI.start()


