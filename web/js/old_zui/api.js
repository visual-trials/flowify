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

var ZUI = function () {
    "use strict"

    var my = {}

    my.createNewWorld = function () {
        return ZUI.world.createNewWorld()
    }

    my.clearWorldContent = function (world) {
        return ZUI.world.clearWorldContent(world)
    }

    // World building API
    my.setContainerTypeProperties = function (world, containerTypeProperties) {
        world.containerTypeProperties = containerTypeProperties
    }

    my.setConnectionTypeProperties = function (world, connectionTypeProperties) {
        world.connectionTypeProperties = connectionTypeProperties
    }

    my.addWorldContainer = function (world, parentContainer, containerIdentifier, containerType, overrulingContainerProperties) {
        return ZUI.world.addWorldContainer(world, parentContainer, containerIdentifier, containerType, overrulingContainerProperties)
    }

    my.addWorldConnection = function (world, fromContainerIdentifier, toContainerIdentifier, connectionType, overrulingConnectionProperties) {
        return ZUI.world.addWorldConnection(world, fromContainerIdentifier, toContainerIdentifier, connectionType, overrulingConnectionProperties)
    }

    my.setReloadWorldFunction = function (world, reloadWorldFunction) {
        world.reloadWorld = reloadWorldFunction
    }

    my.setMustReloadWorld = function (world, mustReloadWorld) {
        world.mustReloadWorld = mustReloadWorld
    }

    my.setInitialContainerIdentifierToCenterOn = function (world, containerIdentifier) {
        world.initialContainerIdentifierToCenterOn = containerIdentifier
    }

    // Input API
    my.addInputListeners = function () {
        ZUI.input.addInputListeners()
    }

    my.addClipboard = function () {
        ZUI.input.addClipboard()
    }

    // Outside listeners API

    // FIXME: rename to setSliceContainerHoverInOutputListener
    my.setContainerHoverInOutputListener = function (world, sliceCcontainerHoverInOutputListener) { // FIXME: first parameter should be a camera!
        world.sliceContainerHoverInCallback = sliceCcontainerHoverInOutputListener
    }

    // FIXME: rename to setSliceContainerHoverOutOutputListener
    my.setContainerHoverOutOutputListener = function (world, sliceContainerHoverOutOutputListener) { // FIXME: first parameter should be a camera!
        world.sliceContainerHoverOutCallback = sliceContainerHoverOutOutputListener
    }

    my.setContainerHasMovedOutputListener = function (world, containerHasMovedOutputListener) { // FIXME: first parameter should be a camera!
        world.sliceContainerWasMovedCallback = containerHasMovedOutputListener
    }

    my.setContainerWasSelectedOutputListener = function (world, containerWasSelectedOutputListener) { // FIXME: first parameter should be a camera!
        world.sliceContainerWasSelectedCallback = containerWasSelectedOutputListener
    }

    my.setContainerWasDoubleClickedOutputListener = function (world, containerWasDoubleClickedOutputListener) { // FIXME: first parameter should be a camera!
        world.sliceContainerWasDoubleClickedCallback = containerWasDoubleClickedOutputListener
    }

    my.setEditModeToggleable = function (world, editModeToggleable) {
        world.editModeToggleable = editModeToggleable
    }

    // Canvas API
    my.setFixedCanvasSize = function (fixedCanvasSize) {
        ZUI.canvas.fixedCanvasSize = fixedCanvasSize
    }

    // Main API
    my.configureInterface = function (configuration) {
        // TODO: this should be extended!
        ZUI.main.worlds = configuration.worlds
    }

    my.start = function () {
        ZUI.main.mainLoop()
    }

    my.log = function (logText) {
        ZUI.canvas.log(logText)
    }

    my.truncateLog = function () {
        ZUI.canvas.truncateLog()
    }

    return my

}()

ZUI.apps = {}
ZUI.callbacks = {} // can be used for JSONP callbacks