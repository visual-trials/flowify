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

Flowify.main = function () {
    "use strict"

    let my = {}
    
    my.autoReload = false
    
    my.mainLoop = function () {

        let currentTime = new Date()

        Flowify.canvas.resizeCanvas()
        Flowify.canvas.clearCanvas()

        let input = Flowify.input
        
        my.wasmInstance.exports._set_left_mouse_button_data(
            input.leftMouseButtonIsDown, input.leftMouseButtonHasGoneUp,
            input.leftMouseButtonHasGoneDown, input.leftMouseButtonHasGoneDownTwice
        )
        my.wasmInstance.exports._set_right_mouse_button_data(
            input.rightMouseButtonIsDown, input.rightMouseButtonHasGoneUp,
            input.rightMouseButtonHasGoneDown, input.rightMouseButtonHasGoneDownTwice
        )
        my.wasmInstance.exports._set_mouse_wheel_data(
            input.mouseWheelHasMoved, input.mouseWheelDelta
        )
        my.wasmInstance.exports._set_mouse_position_data(
            input.mouseHasMoved, input.mousePositionLeft, input.mousePositionTop
        )

        // Update world
        my.wasmInstance.exports._update_frame()
        
        // Render world
        my.wasmInstance.exports._render_frame()

        Flowify.input.resetMouseData()
        Flowify.input.resetTouchData()
        Flowify.input.resetKeyboardData()

        Flowify.canvas.requestAnimFrame(my.mainLoop)
    }
    
    let wasmEnv = {
        memoryBase: 0,
        tableBase: 0,
        memory: new WebAssembly.Memory({
            initial: 256,
            // maximum: 512,
        }),
        table: new WebAssembly.Table({
            initial: 40, // FIXME: can we set/grow this automatically?
            // maximum: 40,
            element: 'anyfunc',
        }),
        abort: Math.log, // FIXME
    }
    
    my.bufferU8 = new Uint8Array(wasmEnv.memory.buffer)
    
    let exportedFunctions = Flowify.canvas.getExportedFunctions()
    for (let functionName in exportedFunctions) {
        wasmEnv[functionName] = exportedFunctions[functionName]; 
    }
    
    let wasmFile = 'wasm/flowify.wasm'
    let latestChangeToWasm = null

    let request = new XMLHttpRequest()
    request.onload = function() {
    
        let wasmCode = request.response
        let responseHeaders = request.getAllResponseHeaders().split("\n")
        
        latestChangeToWasm = responseHeaders[1] // FIXME: ugly way of getting the latest changed time
        
        WebAssembly.instantiate(wasmCode, { env: wasmEnv })
        .then( wasm_module => {
            
            my.wasmInstance = wasm_module.instance
            
            my.wasmInstance.exports._init_world()

            Flowify.input.addInputListeners()
            
            my.mainLoop()
            
        })
    }
    request.open('GET', wasmFile)
    request.responseType = 'arraybuffer'
    request.send()
    
    if (my.autoReload) {
        let reloadWhenWasmChanged = function() {
        
            let request = new XMLHttpRequest()
            request.open('HEAD', wasmFile)
            request.setRequestHeader('cache-control', 'no-cache, must-revalidate, post-check=0, pre-check=0');
            request.send()

            request.onload = function() {
                let responseHeaders = request.getAllResponseHeaders().split("\n")
                
                let newChangeToWasm = responseHeaders[1] // FIXME: ugly way of getting the latest changed time
                // console.log('checking wasm: ' + newChangeToWasm + " -- " + latestChangeToWasm)
                if (latestChangeToWasm != null && newChangeToWasm !== latestChangeToWasm) {
                    location.reload()
                }
                else {
                    setTimeout(reloadWhenWasmChanged, 500)
                }
            }
        }
        
        setTimeout(reloadWhenWasmChanged, 500)
    }

    return my

}()
