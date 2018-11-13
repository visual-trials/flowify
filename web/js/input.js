/*

   Copyright 2018 Jeffrey Hullekes

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

Flowify.input = function () {

    let my = {}

    // Note: we have a single thread in javascript. So we can *record* events in the 
    // variables below and *read* them in the main loop, without them interfering.
    

    // -- Mouse data --

    my.leftMouseButtonIsDown = false
    my.leftMouseButtonHasGoneUp = false
    my.leftMouseButtonHasGoneDown = false
    my.leftMouseButtonHasGoneDownAt = null
    my.leftMouseButtonHasGoneDownTwice = false
    
    my.rightMouseButtonIsDown = false
    my.rightMouseButtonHasGoneUp = false
    my.rightMouseButtonHasGoneDown = false
    my.rightMouseButtonHasGoneDownAt = null
    my.rightMouseButtonHasGoneDownTwice = false
    
    my.mouseWheelHasMoved = false
    my.mouseWheelDelta = null

    // TODO: my.mouseHasEntered
    // TODO: my.mouseHasExited
    
    my.mouseHasMoved = false
    my.mousePositionLeft = null
    my.mousePositionTop = null
    my.lastMouseCoords = []

    // We reset all mouse 'events' every frame (when 'Has' is in the name of the variable, meaning it has happened during the single frame).
    // We do not reset mouse 'states' here (mostly when 'Is' or 'Position' is in the name of the variable).
    my.resetMouseData = function() {
        my.leftMouseButtonHasGoneUp = false
        my.leftMouseButtonHasGoneDown = false
        my.leftMouseButtonHasGoneDownTwice = false
        
        my.rightMouseButtonHasGoneUp = false
        my.rightMouseButtonHasGoneDown = false
        my.rightMouseButtonHasGoneDownTwice = false
        
        my.mouseWheelHasMoved = false

        my.mouseHasMoved = false
        my.lastMouseCoords = [ my.mousePositionLeft, my.mousePositionTop ]
    }
    
    my.sendMouseData = function () {
        Flowify.main.wasmInstance.exports._set_left_mouse_button_data(
            my.leftMouseButtonIsDown, my.leftMouseButtonHasGoneUp,
            my.leftMouseButtonHasGoneDown, my.leftMouseButtonHasGoneDownTwice
        )
        Flowify.main.wasmInstance.exports._set_right_mouse_button_data(
            my.rightMouseButtonIsDown, my.rightMouseButtonHasGoneUp,
            my.rightMouseButtonHasGoneDown, my.rightMouseButtonHasGoneDownTwice
        )
        Flowify.main.wasmInstance.exports._set_mouse_wheel_data(
            my.mouseWheelHasMoved, my.mouseWheelDelta
        )
        Flowify.main.wasmInstance.exports._set_mouse_position_data(
            my.mouseHasMoved, my.mousePositionLeft, my.mousePositionTop
        )
    }

    // -- Mouse events --

    my.mouseButtonUp = function (e) {
        if (e.button == 0) {
            my.leftMouseButtonHasGoneUp = true
            my.leftMouseButtonIsDown = false
        }
        else if (e.button == 2) {
            my.rightMouseButtonHasGoneUp = true
            my.rightMouseButtonIsDown = false
        }

        e.preventDefault()
    }

    my.mouseButtonDown = function (e) {
        
        let now = Date.now()
        
        if (e.button == 0) {
            my.leftMouseButtonHasGoneDown = true
            my.leftMouseButtonIsDown = true
            
            if (my.leftMouseButtonHasGoneDownAt != null && now - my.leftMouseButtonHasGoneDownAt < 500) {
                my.leftMouseButtonHasGoneDownTwice = true
            }
            my.leftMouseButtonHasGoneDownAt = now
        }
        else if (e.button == 2) {
            my.rightMouseButtonHasGoneDown = true
            my.rightMouseButtonIsDown = true
            
            if (my.rightMouseButtonHasGoneDownAt != null && now - my.rightMouseButtonHasGoneDownAt < 500) {
                my.rightMouseButtonHasGoneDownTwice = true
            }
            my.rightMouseButtonHasGoneDownAt = now
        }

        e.preventDefault()
    }

    my.mouseMoved = function (e) {
        my.mouseHasMoved = true

        my.mousePositionLeft = e.offsetX * Flowify.canvas.scale
        my.mousePositionTop = e.offsetY * Flowify.canvas.scale

        e.preventDefault()
    }

    my.mouseEntered = function (e) {
        // TODO: my.mouseHasEntered = true
        
        my.mousePositionLeft = e.offsetX * Flowify.canvas.scale
        my.mousePositionTop = e.offsetY * Flowify.canvas.scale

        e.preventDefault()
    }

    my.mouseExited = function (e) {
        // TODO: my.mouseHasExited = true

        my.mousePositionLeft = e.offsetX * Flowify.canvas.scale
        my.mousePositionTop = e.offsetY * Flowify.canvas.scale

        e.preventDefault()
    }

    my.mouseWheelMoved = function (e) {
        my.mouseWheelHasMoved = true

        // Cross-browser wheel delta (Mac is much more sensitive)
        // A number between -1 and 1
        my.mouseWheelDelta = Math.max(-1, Math.min(1, (e.wheelDelta / 120 || -e.detail)))

        e.preventDefault()
    }
    
    // -- Keyboard data --

    my.keysThatAreDown = {}
    my.sequenceKeysUpDown = []
        
    // -- Clipboard data --

    my.clipboardTextArea = null
    my.textToCopyFrom = null
    my.textHasComeFromClipboard = false
    my.textComingFromClipboard = null

    my.resetKeyboardData = function() {
        
        my.sequenceKeysUpDown = []
        
        // TODO: maybe add: nr of keys that has gone down (and gone up)? (so you know if you have to do anything at all)

        my.textHasComeFromClipboard = false
        my.textComingFromClipboard = null
    }
    
    my.sendKeyboardData = function() {
        for (let sequenceIndex = 0; sequenceIndex < my.sequenceKeysUpDown.length; sequenceIndex++) {
            let keyUpDownEntry = my.sequenceKeysUpDown[sequenceIndex]
            let keyCode = keyUpDownEntry.keyCode
            Flowify.main.bufferU8[my.addressSequenceKeysUpDown + (sequenceIndex * 2) ] = keyUpDownEntry.isDown
            Flowify.main.bufferU8[my.addressSequenceKeysUpDown + (sequenceIndex * 2) + 1] = keyUpDownEntry.keyCode
        }        
        Flowify.main.wasmInstance.exports._set_sequence_keys_length(
            my.sequenceKeysUpDown.length
        )
    }

    my.keyDown = function (e) {
        
        // Using Key Values ( https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key/Key_Values )
        
        // FIXME: e.keyCode and e.which are deprecated, so we should not use them ( https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent )
        let keyValue = e.key;
        let keyCode = e.keyCode ? e.keyCode : e.which

        if (keyCode <= 255) {
            
            if (my.sequenceKeysUpDown.length < 25) {
                my.sequenceKeysUpDown.push({ "isDown" : true, "keyCode" : keyCode})
            }
            else {
                console.log("ERROR: Too many keys have gone up and down during this frame")
            }
            
            if (!Flowify.main.bufferU8[my.addressKeysThatAreDown + keyCode]) {
                Flowify.main.bufferU8[my.addressKeysThatAreDown + keyCode] = 1;
            }
            else {
                // FIXME: a key is down, but there was already one down. This could be multiple keys pressed at once. Not supported atm.
            }
        }
        else {
            console.log("ERROR: Invalid keyCode (" + keyCode + ") encountered!") 
        }

        // Ctrl-c
        if (keyCode == 67 && e.ctrlKey) {   // FIXME: what is the proper way of checking for the key 'c'?
            if (my.clipboardTextArea != null) {
                if (my.textToCopyFrom != null) {
                    my.clipboardTextArea.value = my.textToCopyFrom
                    my.clipboardTextArea.select()

                    try {
                        let successful = document.execCommand('copy')

                        if (!successful) {
                            console.log('Error: unable to copy to clipboard')
                        }
                        else {
                            // console.log('Successfully copied to clipboard')
                        }
                    } catch (err) {
                        console.log('Error: unable to copy to clipboard')
                    }
                }
                else {
                    // TODO: should we empty the clipboard if there is no text selected?
                }

            }
        }

        // Ctrl-v
        if (keyCode == 86 && e.ctrlKey) {   // FIXME: what is the proper way of checking for the key 'v'?
            if (my.clipboardTextArea != null) {
                // This ensures the copy-pasted text goes into the textarea
                my.clipboardTextArea.select()

                // TODO: you will lose focus for 1 millisecond of the canvas.
                setTimeout(my.getClipboardDataFromTextArea, 1)

                // FIXME: should we ensure to restore the focus towards the canvas? Does this work on all browsers?
            }

        }

        // TODO: whould we prevent the alt-key? e.preventDefault()
    }

    my.getClipboardDataFromTextArea = function () {
        my.textHasComeFromClipboard = true
        my.textComingFromClipboard = my.clipboardTextArea.value

        // FIXME: should we ensure to restore the focus towards the canvas? Does this work on all browsers?
    }

    my.keyUp = function (e) {
        // Using Key Values ( https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key/Key_Values )
        // FIXME: e.keyCode and e.which are deprecated, so we should not use them ( https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent )
        let keyValue = e.key;
        let keyCode = e.keyCode ? e.keyCode : e.which
        
        if (keyCode <= 255) {
            
            if (my.sequenceKeysUpDown.length < 25) {
                my.sequenceKeysUpDown.push({ "isDown" : false, "keyCode" : keyCode})
            }
            else {
                console.log("ERROR: Too many keys have gone up and down during this frame")
            }
            
            if (Flowify.main.bufferU8[my.addressKeysThatAreDown + keyCode]) {
                Flowify.main.bufferU8[my.addressKeysThatAreDown + keyCode] = 0;
            }
            else {
                // FIXME: No key was down, but a key went up. What happened?
            }
        }
        else {
            console.log("ERROR: Invalid keyCode (" + keyCode + ") encountered!") 
        }
        
        // TODO: whould we prevent the alt-key? e.preventDefault()

    }
    
    
    // -- Touch data --

    // Note that IE, Opera, Safari do not support touch!  ( https://developer.mozilla.org/en-US/docs/Web/API/Touch )
    
    my.touches = {}
    
    my.sendTouchData = function () {
        
        let touchIndex = 0
        for (let touch_identifier in my.touches){
            
            let touch =  my.touches[touch_identifier]

            Flowify.main.wasmInstance.exports._set_touch_data(
                touchIndex, 
                touch_identifier,
                touch.hasMoved, 
                touch.hasStarted, 
                touch.hasEnded,
                touch.wasCanceled, 
                touch.positionLeft, 
                touch.positionTop
            )
            
            touchIndex++
        }
        
        Flowify.main.wasmInstance.exports._set_touch_count(touchIndex)
        
    }
    
    my.resetTouchData = function () {

        let touchesToDelete = {}
        for (let touch_identifier in my.touches) {
            
            let touch =  my.touches[touch_identifier]
            
            if (touch.hasEnded || touch.wasCanceled) {
                touchesToDelete[touch_identifier] = true
            }
            touch.hasMoved = false
            touch.hasStarted = false
            // touch.hasEnded = false // this is irrelevant, since we are going to delete it anyway
            // touch.wasCanceled = false // this is irrelevant, since we are going to delete it anyway
        }
        
        for (let touch_identifier in touchesToDelete) {
            delete my.touches[touch_identifier]
        }
    }
    
    // -- Touch events --

    my.touchStarted = function (e) {
        
        let changedTouches = e.changedTouches;
        
        for (let touchIndex = 0; touchIndex < changedTouches.length; touchIndex++) {
            let changedTouch = changedTouches[touchIndex]
            
            let newTouch = {}
            newTouch.isActive = true
            newTouch.identifier = changedTouch.identifier
            newTouch.hasMoved = false
            newTouch.hasStarted = true
            newTouch.hasEnded = false
            newTouch.wasCanceled = false
            newTouch.positionLeft = changedTouch.pageX * Flowify.canvas.scale
            newTouch.positionTop = changedTouch.pageY * Flowify.canvas.scale
            
            my.touches[changedTouch.identifier] = newTouch
            
        }

        e.preventDefault()
    }

    my.touchEnded = function (e) {
        
        let changedTouches = e.changedTouches;
        
        for (let touchIndex = 0; touchIndex < changedTouches.length; touchIndex++) {
            let changedTouch = changedTouches[touchIndex]

            if (my.touches.hasOwnProperty(changedTouch.identifier)) {
                let endedTouch = my.touches[changedTouch.identifier]
                
                endedTouch.hasEnded = true
                // TODO: should we do this?: endedTouch.touchHasStarted = false
                
                if (endedTouch.positionLeft !== changedTouch.pageX * Flowify.canvas.scale || 
                    endedTouch.positionTop !== changedTouch.pageY * Flowify.canvas.scale) {
                        
                    endedTouch.hasMoved = true
                    endedTouch.positionLeft = changedTouch.pageX * Flowify.canvas.scale
                    endedTouch.positionTop = changedTouch.pageY * Flowify.canvas.scale
                }
            }
            else {
                console.log("ERROR: touch ended that did not start!")
            }
            
        }

        e.preventDefault()
    }

    my.touchCanceled = function (e) {
        let changedTouches = e.changedTouches;
        
        for (let touchIndex = 0; touchIndex < changedTouches.length; touchIndex++) {
            let changedTouch = changedTouches[touchIndex]

            if (my.touches.hasOwnProperty(changedTouch.identifier)) {
                let canceledTouch = my.touches[changedTouch.identifier]
                canceled.wasCanceled = true
                // TODO: should we do this?: canceled.touchHasStarted = false
            }
            else {
                console.log("ERROR: touch canceled that did not start!")
            }
            
        }

        e.preventDefault()
    }

    my.touchMoved = function (e) {

        let changedTouches = e.changedTouches;
        
        for (let touchIndex = 0; touchIndex < changedTouches.length; touchIndex++) {
            let changedTouch = changedTouches[touchIndex]
            
            if (my.touches.hasOwnProperty(changedTouch.identifier)) {
                let movedTouch = my.touches[changedTouch.identifier]
                movedTouch.hasMoved = true
                movedTouch.positionLeft = changedTouch.pageX * Flowify.canvas.scale
                movedTouch.positionTop = changedTouch.pageY * Flowify.canvas.scale
            }
            else {
                console.log("ERROR: touch moved that did not start!")
            }
        }

        e.preventDefault()
    }

    my.addInputListeners = function () {
        Flowify.canvas.canvasElement.addEventListener("mousedown", my.mouseButtonDown, false)
        Flowify.canvas.canvasElement.addEventListener("mouseup", my.mouseButtonUp, false)
        Flowify.canvas.canvasElement.addEventListener("mousemove", my.mouseMoved, false)
        // TODO: the mouseenter is not triggered on *page load* for Chrome. It is for FF.
        //       See this link *why* we want to use it: 
        //       https://stackoverflow.com/questions/2601097/how-to-get-the-mouse-position-without-events-without-moving-the-mouse
        Flowify.canvas.canvasElement.addEventListener("mouseenter", my.mouseEntered, false)
        Flowify.canvas.canvasElement.addEventListener("mouseleave", my.mouseExited, false)
        // IE9, Chrome, Safari, Opera
        Flowify.canvas.canvasElement.addEventListener("mousewheel", my.mouseWheelMoved, false)
        // Firefox
        Flowify.canvas.canvasElement.addEventListener("DOMMouseScroll", my.mouseWheelMoved, false)

        document.addEventListener("keydown", my.keyDown, false)
        document.addEventListener("keyup", my.keyUp, false)
        
        Flowify.canvas.canvasElement.addEventListener("touchstart", my.touchStarted, false)
        Flowify.canvas.canvasElement.addEventListener("touchend", my.touchEnded, false)
        Flowify.canvas.canvasElement.addEventListener("touchcancel", my.touchCanceled, false)
        Flowify.canvas.canvasElement.addEventListener("touchmove", my.touchMoved, false)
    }

    my.addClipboard = function () {
        // let body = document.getElementById("body")
        my.clipboardTextArea = document.createElement("textarea");

        my.clipboardTextArea.style="width:100px; height:100px; position: absolute; top: -200px; left: -200px;"

        document.body.appendChild(my.clipboardTextArea);
    }

    my.getCookie = function (cname) {
        let name = cname + "="
        let decodedCookie = decodeURIComponent(document.cookie)
        let ca = decodedCookie.split(';')
        for(let i = 0; i < ca.length; i++) {
            let c = ca[i]
            while (c.charAt(0) === ' ') {
                c = c.substring(1)
            }
            if (c.indexOf(name) === 0) {
                return c.substring(name.length, c.length)
            }
        }
        return ""
    }

    my.storeCookie = function (cookieName, cookieValue) {
        // TODO: do the proper way of storing a cookie!
        document.cookie = cookieName + "=" + cookieValue + ";";
    }

    return my
}()
