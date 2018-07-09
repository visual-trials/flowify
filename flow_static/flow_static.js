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
        paddingTop: 20,
        paddingBottom: 20,
        paddingLeft: 20,
        paddingRight: 20,
        paddingBetweenChildren: 20,
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
        showContainerText: true,
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        childrenLayoutFunction: "none",
    },
    // This function has no input, body and output conatiners (so direct manualPositioning)
    function: {
        drawContainer: true,
        isSelectable: true,   // FIXME: workaround to make it possible to move a functionWrapper!
        showContainerBody: true,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 230, a: 0},
        paddingTop: 15,
        paddingBottom: 15,
        paddingLeft: 15,
        paddingRight: 15,
        childrenLayoutFunction: "manualPositioning",
    },
    /*
       This is a function with would have an input, body and output container ( and horizontalLeftToRight-layout )
    function: {
        drawContainer: true,
        containerColor: {r: 255, g: 255, b: 240, a: 1},
        containerBorderColor: {r: 240, g: 240, b: 220, a: .8},
        paddingTop: 20,
        paddingBottom: 20,
        paddingLeft: 20,
        paddingRight: 20,
        paddingBetweenChildren: 20,
        childrenLayoutFunction: "horizontalLeftToRight",
    },
    */
    primitiveFunctionWrapper: {
        drawContainer: true,
        showContainerBody: false,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 200, a: 0},
        childrenLayoutFunction: "verticalTopToBottom",
    },
    primitiveFunctionHeader: {
        drawContainer: true,
        isSelectable: false,   // FIXME: workaround to make it possible to move a primitiveFunctionWrapper!
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 1},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 1},
        showContainerText: true,
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
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
        connectIncomingTo: "4-directions",
        childrenLayoutFunction: "horizontalLeftToRight",
        dataItemGroups: {
            basicInfo: [
                { containerKey: "identifier", displayName: "identifier" },
                { containerKey: "type", displayName: "type" },
                { containerDataKey: "astNodeIdentifier", displayName: "astNodeIdentifier" }
            ]
        }
    },
    ifMainWrapper: {
        drawContainer: true,
        showContainerBody: false,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 200, a: 0},
        childrenLayoutFunction: "verticalTopToBottom",
    },
    ifMainHeader: {
        drawContainer: true,
        isSelectable: false,   // FIXME: workaround to make it possible to move a ifMainWrapper!
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 1},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 1},
        showContainerText: true,
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        childrenLayoutFunction: "none",
    },
    ifMain: {
        drawContainer: true,
        isSelectable: true,   // FIXME: workaround to make it possible to move a ifMainWrapper!
        showContainerBody: true,
        containerColor: {r: 255, g: 255, b: 240, a: 1},
        containerBorderColor: {r: 240, g: 240, b: 220, a: .8},
        paddingTop: 15,
        paddingBottom: 15,
        paddingLeft: 15,
        paddingRight: 15,
        childrenLayoutFunction: "manualPositioning",
    },
    ifCondWrapper: {
        drawContainer: true,
        showContainerBody: false,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 200, a: 0},
        childrenLayoutFunction: "verticalTopToBottom",
    },
    ifCondHeader: {
        drawContainer: true,
        isSelectable: false,   // FIXME: workaround to make it possible to move a ifCondWrapper!
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 1},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 1},
        showContainerText: true,
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        childrenLayoutFunction: "none",
    },
    ifCond: {
        drawContainer: true,
        isSelectable: true,   // FIXME: workaround to make it possible to move a ifCondWrapper!
        showContainerBody: true,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 230, a: 0},
        paddingTop: 15,
        paddingBottom: 15,
        paddingLeft: 15,
        paddingRight: 15,
        childrenLayoutFunction: "manualPositioning",
    },
    ifThenWrapper: {
        drawContainer: true,
        showContainerBody: false,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 200, a: 0},
        childrenLayoutFunction: "verticalTopToBottom",
    },
    ifThenHeader: {
        drawContainer: true,
        isSelectable: false,   // FIXME: workaround to make it possible to move a ifThenWrapper!
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 1},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 1},
        showContainerText: true,
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        childrenLayoutFunction: "none",
    },
    ifThen: {
        drawContainer: true,
        isSelectable: true,   // FIXME: workaround to make it possible to move a ifThenWrapper!
        showContainerBody: true,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 230, a: 0},
        paddingTop: 15,
        paddingBottom: 15,
        paddingLeft: 15,
        paddingRight: 15,
        childrenLayoutFunction: "manualPositioning",
    },
    ifElseWrapper: {
        drawContainer: true,
        showContainerBody: false,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 200, a: 0},
        childrenLayoutFunction: "verticalTopToBottom",
    },
    ifElseHeader: {
        drawContainer: true,
        isSelectable: false,   // FIXME: workaround to make it possible to move a ifThenWrapper!
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 1},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 1},
        showContainerText: true,
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        childrenLayoutFunction: "none",
    },
    ifElse: {
        drawContainer: true,
        isSelectable: true,   // FIXME: workaround to make it possible to move a ifThenWrapper!
        showContainerBody: true,
        containerColor: {r: 240, g: 240, b: 240, a: 1},
        containerBorderColor: {r: 230, g: 230, b: 230, a: 0},
        paddingTop: 15,
        paddingBottom: 15,
        paddingLeft: 15,
        paddingRight: 15,
        childrenLayoutFunction: "manualPositioning",
    },
    variable: {
        drawContainer: true,
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 1},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 1},
        showContainerText: true,
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        childrenLayoutFunction: "none",
        dataItemGroups: {
            basicInfo: [
                { containerKey: "identifier", displayName: "identifier" },
                { containerKey: "type", displayName: "type" },
                { containerDataKey: "astNodeIdentifier", displayName: "astNodeIdentifier" }
            ]
        }
    },
    passThroughVariable: {
        drawContainer: true,
        showContainerBody: true,
        containerColor: {r: 210, g: 230, b: 255, a: 0.3},
        containerBorderColor: {r: 180, g: 200, b: 255, a: 0.3},
        showContainerText: true,
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        childrenLayoutFunction: "none",
        dataItemGroups: {
            basicInfo: [
                { containerKey: "identifier", displayName: "identifier" },
                { containerKey: "type", displayName: "type" },
                { containerDataKey: "astNodeIdentifier", displayName: "astNodeIdentifier" }
            ]
        }
    },
    conditionalVariable: {
        drawContainer: true,
        showContainerBody: true,
        containerColor: {r: 150, g: 150, b: 150, a: 1},
        containerBorderColor: {r: 150, g: 150, b: 150, a: 1},
        shape: "ellipse",
        showContainerText: false,
        paddingTop: 4,
        paddingBottom: 4,
        paddingLeft: 4,
        paddingRight: 4,
        connectIncomingTo: "3-directions",
        childrenLayoutFunction: "none",
        dataItemGroups: {
            basicInfo: [
                { containerKey: "identifier", displayName: "identifier" },
                { containerKey: "type", displayName: "type" },
                { containerDataKey: "astNodeIdentifier", displayName: "astNodeIdentifier" }
            ]
        }
    },
    conditionalSplitVariable: {
        drawContainer: true,
        showContainerBody: true,
        containerColor: {r: 150, g: 150, b: 150, a: 0},
        containerBorderColor: {r: 150, g: 150, b: 150, a: 1},
        shape: "ellipse",
        showContainerText: false,
        paddingTop: 4,
        paddingBottom: 4,
        paddingLeft: 4,
        paddingRight: 4,
        childrenLayoutFunction: "none",
        dataItemGroups: {
            basicInfo: [
                { containerKey: "identifier", displayName: "identifier" },
                { containerKey: "type", displayName: "type" },
                { containerDataKey: "astNodeIdentifier", displayName: "astNodeIdentifier" }
            ]
        }
    },
    constant: {
        drawContainer: true,
        showContainerBody: true,
        containerColor: {r: 255, g: 230, b: 230, a: 1},
        containerBorderColor: {r: 255, g: 200, b: 180, a: 1},
        showContainerText: true,
        paddingTop: 10,
        paddingBottom: 10,
        paddingLeft: 10,
        paddingRight: 10,
        childrenLayoutFunction: "none",
        dataItemGroups: {
            basicInfo: [
                { containerKey: "identifier", displayName: "identifier" },
                { containerKey: "type", displayName: "type" },
                { containerDataKey: "astNodeIdentifier", displayName: "astNodeIdentifier" }
            ]
        }
    },
}
var connectionTypeProperties = {
    dataFlow: {
    }
}

var queryParams = (function () {
	'use strict'

	var qs = document.location.search.substring(1)
	var parsed = {}
	var parts = qs.split('&')

	var i, len, indexOfEq, key, val
	for (i = 0, len = parts.length; i < len; i += 1) {
		indexOfEq = parts[i].indexOf('=')

		if (indexOfEq > -1) {
			key = decodeURIComponent(parts[i].substring(0, indexOfEq))
			val = decodeURIComponent(parts[i].substring(indexOfEq + 1))
		}
		else {
			key = decodeURIComponent(parts[i])
			val = true
		}

		parsed[key] = val
	}

	return parsed
}())

// TODO: maybe put this in a separate function!
var fileToFlowify = null
if (queryParams['fileToFlowify']) {
	fileToFlowify = queryParams['fileToFlowify']
}
else {
	fileToFlowify = 'simpleMath.php'
}
var url = "flowifyPHP.php?fileToFlowify=examples/" + fileToFlowify
	
var reloadWorld = function (world) {

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

var extendWorldContainerWithVisualInfo = function (worldContainer, flowElement) {
    if (flowElement.hasOwnProperty("x")) {
        worldContainer.manualPosition.x = flowElement.x
    }
    if (flowElement.hasOwnProperty("y")) {
        worldContainer.manualPosition.y = flowElement.y
    }
    if (flowElement.hasOwnProperty("isPositionOf")) {
        worldContainer.manualPosition.isPositionOf = flowElement.isPositionOf
    }
    else {
        worldContainer.manualPosition.isPositionOf = "center"  // FIXME: should this not simply be stored? (and not rely on the default of the engine! or some hardcoded value here!)
    }
    if (flowElement.hasOwnProperty("width")) {
        worldContainer.manualSize.width = flowElement.width
    }
    if (flowElement.hasOwnProperty("height")) {
        worldContainer.manualSize.height = flowElement.height
    }
    if (flowElement.hasOwnProperty("relativeScale")) {
        worldContainer.manualRelativeScale = flowElement.relativeScale
    }

    worldContainer.containerData.astNodeIdentifier = flowElement.astNodeIdentifier
}

function convertFlowDataToZUIContainers (world, flowData) {

    // TODO: a connection should have a type, most notably:

    // 1) Identity (this is the same element)
    // 2) Reference (this is a reference to the element)
    // 3) Copy (this is copy of the element)
    // 4) Conversion (this is the element, changed by some operation)

    var addContainersToWorld = function (world, parentWorldContainer, flowElement) {

        var containerType = flowElement.type  // FIXME: check if it exists!

        var overrulingContainerProperties = {}
        var containerText = null

        var addHeader = false
        var overrulingHeaderProperties = {}
        var headerText = null

        if (flowElement.type === 'variable') {
            containerText = flowElement.name
        }
        else if (flowElement.type === 'passThroughVariable') {
            containerText = flowElement.name
        }
        else if (flowElement.type === 'conditionalVariable') {
            containerText = flowElement.name
        }
        else if (flowElement.type === 'conditionalSplitVariable') {
            containerText = flowElement.name
        }
        else if (flowElement.type === 'constant') {
            containerText = flowElement.value
        }
        else if (flowElement.type === 'ifCond') {
            addHeader = true
            headerText = flowElement.name
        }
        else if (flowElement.type === 'ifThen') {
            addHeader = true
            headerText = flowElement.name
        }
        else if (flowElement.type === 'ifElse') {
            addHeader = true
            headerText = flowElement.name
        }
        else if (flowElement.type === 'ifMain') {
            addHeader = true
            // containerText = flowElement.name
            headerText = flowElement.name
        }
        else if (flowElement.type === 'primitiveFunction') {
            addHeader = true
            headerText = flowElement.name
        }
        else if (flowElement.type === 'function') {
            addHeader = true
            headerText = flowElement.name
        }
        else if (flowElement.type === 'root') {
            containerText = 'root'
        }

        if (addHeader) {
            var worldContainerWrapper = ZUI.addWorldContainer(world, parentWorldContainer, flowElement.id + '_Wrapper', containerType + 'Wrapper')

            // FIXME: we are adding position AND size here, but we only need to add one of them!
            extendWorldContainerWithVisualInfo(worldContainerWrapper, flowElement)

            parentWorldContainer = worldContainerWrapper

            // FIXME: we are accessing 'containerTypeProperties' here. We should do this here AND let world.js do the same. We should pass it through the addWorldContainer-function and don't set it through the setContainerTypeProperties()
            var containerProperties = containerTypeProperties[containerType]
            overrulingHeaderProperties.containerColor = containerProperties.containerColor
            overrulingHeaderProperties.containerBorderColor = containerProperties.containerBorderColor

            overrulingHeaderProperties.containerText = headerText
            var worldContainerHeader = ZUI.addWorldContainer(world, parentWorldContainer, flowElement.id + '_Header', containerType + 'Header', overrulingHeaderProperties)

            overrulingContainerProperties.containerColor = {r: 250, g: 250, b: 250, a: 1}
            overrulingContainerProperties.containerBorderColor = {r: 150, g: 150, b: 150, a: 0.1}
            overrulingContainerProperties.containerText = '' // FIXME: simply don't draw the text?
            // overrulingContainerProperties.containerText = containerText

            var worldContainer = ZUI.addWorldContainer(world, parentWorldContainer, flowElement.id, containerType, overrulingContainerProperties)

            // FIXME: we are adding position AND size here, but we only need to add one of them!
            extendWorldContainerWithVisualInfo(worldContainer, flowElement)
            // TODO: resetting the manualRelativeScale to one, since the wrapper already got the relativeScale (is there a nicer way of doing this?)
            worldContainer.manualRelativeScale = 1

        }
        else {
            overrulingContainerProperties.containerText = containerText
            
            var worldContainer = ZUI.addWorldContainer(world, parentWorldContainer, flowElement.id, containerType, overrulingContainerProperties)

            extendWorldContainerWithVisualInfo(worldContainer, flowElement)

        }

        if (flowElement.hasOwnProperty('children')) {
            for (var loopIndex = 0; loopIndex < flowElement.children.length; loopIndex++) {
                var childFlowElement = flowElement.children[loopIndex]
                addContainersToWorld(world, worldContainer, childFlowElement)
            }
        }
    }

    ZUI.clearWorldContent(world)

    var rootFlowElement = flowData.rootFlowElement
    var flowConnections = flowData.flowConnections

    addContainersToWorld(world, null, rootFlowElement)

    for (loopIndex = 0; loopIndex < flowConnections.length; loopIndex++) {
        var flowConnection = flowConnections[loopIndex]

        var overrulingConnectionProperties = {}

        // FIXME: these should be properties of the connnectionType
        overrulingConnectionProperties['lineNormalizedWidth'] = 2;
        overrulingConnectionProperties['arrowPointNormalizedLength'] = 12;
        
        if (flowConnection.type === 'conditional') {
            overrulingConnectionProperties['dashStyle'] = [5,2]
        }
        if (flowConnection.type === 'identity') {
            overrulingConnectionProperties['dashStyle'] = [1,5]
        }

        // TODO: connection.type?
        ZUI.addWorldConnection(world, flowConnection.from, flowConnection.to, 'dataFlow', overrulingConnectionProperties)
    }

}


var sliceContainerWasMoved = function (movedSliceContainer) {
    console.log("Container was moved:")
    console.log(movedSliceContainer)

    var visualInfos = {}

    // FIXME: we are assuming astNodeIdentifier is always present! But this is not always the case! (we are not allowed to move it then?)
    var astNodeIdentifier = movedSliceContainer.worldContainer.containerData.astNodeIdentifier
    if (astNodeIdentifier != null) {
        visualInfos[astNodeIdentifier] = {}
        if (movedSliceContainer.worldContainer.manualPosition.x != null) {
            visualInfos[astNodeIdentifier]["x"] = movedSliceContainer.worldContainer.manualPosition.x
        }
        if (movedSliceContainer.worldContainer.manualPosition.y != null) {
            visualInfos[astNodeIdentifier]["y"] = movedSliceContainer.worldContainer.manualPosition.y
        }
        if (movedSliceContainer.worldContainer.manualPosition.isPositionOf != null) {
            visualInfos[astNodeIdentifier]["isPositionOf"] = movedSliceContainer.worldContainer.manualPosition.isPositionOf
        }
        if (movedSliceContainer.worldContainer.manualSize.width != null) {
            visualInfos[astNodeIdentifier]["width"] = movedSliceContainer.worldContainer.manualSize.width
        }
        if (movedSliceContainer.worldContainer.manualSize.height != null) {
            visualInfos[astNodeIdentifier]["height"] = movedSliceContainer.worldContainer.manualSize.height
        }
        if (movedSliceContainer.worldContainer.manualRelativeScale != null) {
            visualInfos[astNodeIdentifier]["relativeScale"] = movedSliceContainer.worldContainer.manualRelativeScale
        }

        var xmlhttp = new XMLHttpRequest()
        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                var setPositionResult = JSON.parse(xmlhttp.responseText)

                // TODO: do something with the result!
                // console.log(setPositionResult)
            }
        }

        var formData = "visualInfos=" + JSON.stringify(visualInfos)
        xmlhttp.open("POST", url, true)
        xmlhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded")
        xmlhttp.send(formData)
        // FIXME: we should use this! xmlhttp.setRequestHeader("Content-Type", "application/json")
        // FIXME: we should use this! xmlhttp.send(JSON.stringify(visualInfos))

    }
}

var sliceContainerWasSelected = function (selectedSliceContainer) {
    // console.log(selectedSliceContainer)
}

var flowWorld = ZUI.world.createNewWorld()
ZUI.setContainerTypeProperties(flowWorld, containerTypeProperties)
ZUI.setConnectionTypeProperties(flowWorld, connectionTypeProperties)
ZUI.setReloadWorldFunction(flowWorld, reloadWorld)
// ZUI.setContainerWasDoubleClickedOutputListener(flowWorld, sliceContainerWasDoubleClicked)
ZUI.setContainerHasMovedOutputListener(flowWorld, sliceContainerWasMoved)
ZUI.setContainerWasSelectedOutputListener(flowWorld, sliceContainerWasSelected)
ZUI.setEditModeToggleable(flowWorld, true)

ZUI.configureInterface(
    {
        worlds: {
            main: flowWorld
        }
    }
)
ZUI.addInputListeners()
ZUI.start()


