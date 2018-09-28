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

ZUI.input = function () {

    var my = {}

    // Note: we have ONE THREAD in javascript! So we can record events in these variables and read them from the main loop, without them interfering.
    //       Currently it is however limited to recording one type of an event per frame. We might want to extend this later on.

    // -- Touch data --

    my.touchHasMoved = false
    my.touchHasStarted = false
    my.touchHasEnded = false

    my.oneTouchActive = false
    my.twoTouchesActive = false

    my.previousTouchDistance = 0
    my.previousTouchCoords = []

    // FIXME: use Touch.identifier!!
    my.firstTouchLeftPx = null
    my.firstTouchTopPx = null
    my.secondTouchLeftPx = null
    my.secondTouchTopPx = null

    my.resetTouchData = function () {

        // TODO: we are NOT recording the lastTouchCoords using e.changedTouches right now! (during TouchEnd)
        if ((my.touchHasStarted || my.touchHasMoved) && my.oneTouchActive) {
            my.previousTouchCoords = [ my.firstTouchLeftPx, my.firstTouchTopPx ]
        }

        my.touchHasMoved = false
        my.touchHasStarted = false
        my.touchHasEnded = false
    }

    // -- Mouse data --

    my.mouseHasMoved = false
    my.mouseButtonHasGoneUp = false
    my.mouseButtonHasGoneDown = false
    my.mouseButtonHasGoneDownTime = null
    my.mouseButtonHasGoneDownTwice = false
    my.mouseWheelHasChanged = false

    // FIXME: record up/down per button
    my.mouseLeftButtonIsDown = false
    my.lastMouseCoords = []

    my.mouseLeftPx = null
    my.mouseTopPx = null
    my.mouseWheelDelta = null

    my.resetMouseData = function() {
        // FIXME: record up/down per button
        my.mouseHasMoved = false
        my.mouseButtonHasGoneUp = false
        my.mouseButtonHasGoneDown = false
        my.mouseButtonHasGoneDownTwice = false
        my.mouseWheelHasChanged = false

        my.lastMouseCoords = [ my.mouseLeftPx, my.mouseTopPx ]
    }


    // -- Keyboard data --

    // TODO: multiple pressed keys? (apart from ctrl/shift/alt keys)

    // FIXME: what if a sequence of key presses have been made? Should we keep a record here? Since we might not run the main update function before that!
    // my.keysHaveBeenPressed = false
    // my.keysThatHaveBeenPressed = []

    my.ctrlKeyIsDown = false
    my.ctrlKeyHasGoneDown = false
    my.ctrlKeyHasGoneUp = false

    my.shiftKeyIsDown = false
    my.shiftKeyHasGoneDown = false
    my.shiftKeyHasGoneUp = false

    my.altKeyIsDown = false
    my.altKeyHasGoneDown = false
    my.altKeyHasGoneUp = false

    my.keyIsDown = false
    my.keyHasGoneDown = false
    my.keyHasGoneUp = false
    my.keyThatIsDown = null

    // -- Clipboard data --

    my.clipboardTextArea = null
    my.textToCopyFrom = null
    my.textHasComeFromClipboard = false
    my.textComingFromClipboard = null

    my.resetKeyboardData = function() {
        my.ctrlKeyHasGoneDown = false
        my.ctrlKeyHasGoneUp = false

        my.shiftKeyHasGoneDown = false
        my.shiftKeyHasGoneUp = false

        my.altKeyHasGoneDown = false
        my.altKeyHasGoneUp = false

        my.keyHasGoneDown = false
        my.keyHasGoneUp = false

        my.textHasComeFromClipboard = false
        my.textComingFromClipboard = null
    }

    my.keyDown = function (e) {
        var key = e.keyCode ? e.keyCode : e.which;
        // FIXME: should we use this? var ctrl = e.ctrlKey ? e.ctrlKey : ((key === 17) ? true : false) // ctrl detection

        if (e.ctrlKey) {
            if (!my.ctrlKeyIsDown) {
                my.ctrlKeyIsDown = true
                my.ctrlKeyHasGoneDown = true
            }
            else {
                // A key has gone down, but it wasn't the ctrl-key (since it was already down)
            }
        }
        else if (e.altKey) {
            if (!my.altKeyIsDown) {
                my.altKeyIsDown = true
                my.altKeyHasGoneDown = true
            }
            else {
                // A key has gone down, but it wasn't the alt-key (since it was already down)
            }
        }
        else if (e.shiftKey) {
            if (!my.shiftKeyIsDown) {
                my.shiftKeyIsDown = true
                my.shiftKeyHasGoneDown = true
            }
            else {
                // A key has gone down, but it wasn't the shift-key (since it was already down)
            }
        }

        if (!my.keyIsDown) {
            my.keyIsDown = true
            my.keyHasGoneDown = true
            my.keyThatIsDown = key

            // FIXME: we probably want to record all key-downs until the main-loop-function runs again...
        }
        else {
            // FIXME: a key is down, but there was already one down. This could be multiple keys pressed at once. Not supported atm.
        }

        // Ctrl-c
        if (key == 67 && e.ctrlKey) {   // FIXME: what is the proper way of checking for the key 'c'?
            if (my.clipboardTextArea != null) {
                if (my.textToCopyFrom != null) {
                    my.clipboardTextArea.value = my.textToCopyFrom
                    my.clipboardTextArea.select()

                    try {
                        var successful = document.execCommand('copy')

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
        if (key == 86 && e.ctrlKey) {   // FIXME: what is the proper way of checking for the key 'v'?
            if (my.clipboardTextArea != null) {
                // This ensures the copy-pasted text goes into the textarea
                my.clipboardTextArea.select()

                // TODO: you will lose focus for 1 millisecond of the canvas.
                setTimeout(my.getClipboardDataFromTextArea, 1)

                // FIXME: should we ensure to restore the focus towards the canvas? Does this work on all browsers?
            }

        }

    }

    my.getClipboardDataFromTextArea = function () {
        my.textHasComeFromClipboard = true
        my.textComingFromClipboard = my.clipboardTextArea.value

        // FIXME: should we ensure to restore the focus towards the canvas? Does this work on all browsers?
    }

    my.keyUp = function (e) {
        var key = e.keyCode ? e.keyCode : e.which;

        if (e.ctrlKey) {
            if (my.ctrlKeyIsDown) {
                my.ctrlKeyIsDown = false
                my.ctrlKeyHasGoneUp = true
            }
            else {
                // A key has gone up, but it wasn't the ctrl-key (since it was not down)
            }
        }
        else if (e.altKey) {
            if (my.altKeyIsDown) {
                my.altKeyIsDown = false
                my.altKeyHasGoneUp = true
            }
            else {
                // A key has gone up, but it wasn't the alt-key (since it was not down)
            }
        }
        else if (e.shiftKey) {
            if (my.shiftKeyIsDown) {
                my.shiftKeyIsDown = false
                my.shiftKeyHasGoneUp = true
            }
            else {
                // A key has gone up, but it wasn't the alt-key (since it was not down)
            }
        }

        if (my.keyIsDown) {
            my.keyIsDown = false
            my.keyHasGoneUp = true
            my.keyThatIsDown = null

            // TODO: record what key went up?
        }
        else {
            // FIXME: No key was down, but a key went up. What happened?

        }

    }

    // -- Touch events --

    my.touchStarted = function (e) {
        my.touchHasStarted = true

        if (e.touches.length === 1) {
            var touch = e.touches[0]

            my.oneTouchActive = true
            my.twoTouchesActive = false

            my.firstTouchLeftPx = touch.pageX
            my.firstTouchTopPx = touch.pageY

            my.secondTouchLeftPx = null
            my.secondTouchTopPx = null
        }
        else if (e.touches.length === 2) {
            var touch1 = e.touches[0]
            var touch2 = e.touches[1]

            my.oneTouchActive = false
            my.twoTouchesActive = true

            my.firstTouchLeftPx = touch1.pageX
            my.firstTouchTopPx = touch1.pageY

            my.secondTouchLeftPx = touch2.pageX
            my.secondTouchTopPx = touch2.pageY
        }
        else {
            // TODO
        }

        e.preventDefault()
    }

    my.touchEnded = function (e) {
        my.touchHasEnded = true

        if (e.touches.length === 1) {
            var touch = e.touches[0]

            my.oneTouchActive = true
            my.twoTouchesActive = false

            my.firstTouchLeftPx = touch.pageX
            my.firstTouchTopPx = touch.pageY

            // TODO: this is a bit of a workaround/hack:
            // Since we could have 'switched' the firstTouch (if we let go the actual first touch, the second touch now has become the first).
            // So we better make sure the previousTouchCoords (which in that case is still from the actual first touch) is replaced by the NEW first touch (which was the actual second touch).
            // This prevents 'jumping around' when letting go of the actual first touch (and moving the new first touch)
            my.previousTouchCoords = [ my.firstTouchLeftPx, my.firstTouchTopPx ]

            my.secondTouchLeftPx = null
            my.secondTouchTopPx = null

        }
        else if (e.touches.length === 0) {
            my.oneTouchActive = false
            my.twoTouchesActive = false

            my.firstTouchLeftPx = null
            my.firstTouchTopPx = null

            my.secondTouchLeftPx = null
            my.secondTouchTopPx = null
        }
        else {
            // TODO
        }

        e.preventDefault()
    }

    my.touchMoved = function (e) {
        my.touchHasMoved = true

        if (e.touches.length === 1) {
            var touch = e.touches[0]

            my.oneTouchActive = true
            my.twoTouchesActive = false

            my.firstTouchLeftPx = touch.pageX
            my.firstTouchTopPx = touch.pageY

            my.secondTouchLeftPx = null
            my.secondTouchTopPx = null
        }
        else if (e.touches.length === 2) {
            var touch1 = e.touches[0]
            var touch2 = e.touches[1]

            my.oneTouchActive = false
            my.twoTouchesActive = true

            my.firstTouchLeftPx = touch1.pageX
            my.firstTouchTopPx = touch1.pageY

            my.secondTouchLeftPx = touch2.pageX
            my.secondTouchTopPx = touch2.pageY
        }
        else {
            // TODO
        }

        e.preventDefault()
    }

    my.touchCanceled = function (e) {
        // TODO: implement this?

        e.preventDefault()
    }


    // -- Mouse events --

    // FIXME: record current up/down-state for EACH button!

    my.mouseButtonUp = function (e) {
        my.mouseButtonHasGoneUp = true
        my.mouseLeftButtonIsDown = false

        my.mouseLeftPx = e.offsetX
        my.mouseTopPx = e.offsetY

        e.preventDefault()
    }

    my.mouseButtonDown = function (e) {
        my.mouseButtonHasGoneDown = true

        var now = Date.now()
        if (my.mouseButtonHasGoneDownTime != null && now - my.mouseButtonHasGoneDownTime < 500) {
            my.mouseButtonHasGoneDownTwice = true
        }
        my.mouseButtonHasGoneDownTime = now

        my.mouseLeftButtonIsDown = true

        my.mouseLeftPx = e.offsetX
        my.mouseTopPx = e.offsetY

        e.preventDefault()
    }

    my.mouseMoved = function (e) {
        my.mouseHasMoved = true

        my.mouseLeftPx = e.offsetX
        my.mouseTopPx = e.offsetY

        var mouseNrOfButtonsPressed = e.buttons

        // If no more buttons are being pressed and the button was down (but we apparently didn't get a mouseButtonUp signal)
        // we assume the mouseButtonHasGoneUp
        if (mouseNrOfButtonsPressed === 0 && my.mouseLeftButtonIsDown) {
            my.mouseLeftButtonIsDown = false
            my.mouseButtonHasGoneUp = true
        }
        // FIXME: if left mouse button is pressed (determined using e.buttons?) but mouseLeftButtonIsDown is still false, shouldn't we set it to true? And should we set mouseButtonHasGoneDown to true?

        e.preventDefault()
    }

    my.mouseWheelChanged = function (e) {
        my.mouseWheelHasChanged = true

        my.mouseLeftPx = e.offsetX
        my.mouseTopPx = e.offsetY

        // cross-browser wheel delta
        my.mouseWheelDelta = Math.max(-1, Math.min(1, (e.wheelDelta / 120 || -e.detail)))

        e.preventDefault()
        e.preventDefault()
    }

    my.addInputListeners = function () {
        ZUI.canvas.canvasElement.addEventListener("touchstart", my.touchStarted, false)
        ZUI.canvas.canvasElement.addEventListener("touchend", my.touchEnded, false)
        ZUI.canvas.canvasElement.addEventListener("touchcancel", my.touchCanceled, false)
        ZUI.canvas.canvasElement.addEventListener("touchmove", my.touchMoved, false)
        ZUI.canvas.canvasElement.addEventListener("mousedown", my.mouseButtonDown, false)
        ZUI.canvas.canvasElement.addEventListener("mouseup", my.mouseButtonUp, false)
        ZUI.canvas.canvasElement.addEventListener("mousemove", my.mouseMoved, false)
        // IE9, Chrome, Safari, Opera
        ZUI.canvas.canvasElement.addEventListener("mousewheel", my.mouseWheelChanged, false)
        // Firefox
        ZUI.canvas.canvasElement.addEventListener("DOMMouseScroll", my.mouseWheelChanged, false)

        document.addEventListener("keydown", my.keyDown, false)
        document.addEventListener("keyup", my.keyUp, false)
    }

    my.addClipboard = function () {
        // var body = document.getElementById("body")
        my.clipboardTextArea = document.createElement("textarea");

        my.clipboardTextArea.style="width:100px; height:100px; position: absolute; top: -200px; left: -200px;"

        document.body.appendChild(my.clipboardTextArea);
    }

    my.getCookie = function (cname) {
        var name = cname + "="
        var decodedCookie = decodeURIComponent(document.cookie)
        var ca = decodedCookie.split(';')
        for(var i = 0; i < ca.length; i++) {
            var c = ca[i]
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
