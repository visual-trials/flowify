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

    var my = {}
    
    my.autoReload = false
    
    my.test = 0 // FIXME: remove this

    my.mainLoop = function () {

        var currentTime = new Date()

        Flowify.canvas.resizeCanvas()

        var input = Flowify.input
        
        // console.log("left mouse button is down: " + input.leftMouseButtonIsDown)
        my.wasmInstance.exports._set_left_mouse_button_data(
            input.leftMouseButtonIsDown, input.leftMouseButtonHasGoneUp,
            input.leftMouseButtonHasGoneDown, input.leftMouseButtonHasGoneDownTwice
        )
            
        // TODO: here should the input, update and render take place
        my.test++
        my.wasmInstance.exports._draw_frame(my.test)

        Flowify.input.resetMouseData()
        Flowify.input.resetTouchData()
        Flowify.input.resetKeyboardData()

        Flowify.canvas.requestAnimFrame(my.mainLoop)
    }
    
    var wasmEnv = {
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
    
    var exportedFunctions = Flowify.canvas.getExportedFunctions()
    for (var functionName in exportedFunctions) {
        wasmEnv[functionName] = exportedFunctions[functionName]; 
    }
    
    var wasmFile = 'wasm/flowify.wasm'
    var latestChangeToWasm = null
    
    var request = new XMLHttpRequest()
    request.onload = function() {
    
        var wasmCode = request.response
        var responseHeaders = request.getAllResponseHeaders().split("\n")
        
        latestChangeToWasm = responseHeaders[1] // FIXME: ugly way of getting the latest changed time
        
        var wasmModule = new WebAssembly.Module(wasmCode)
        my.wasmInstance = new WebAssembly.Instance(wasmModule, {
            env: wasmEnv
        })
        
        Flowify.input.addInputListeners()

        my.mainLoop()
    }
    request.open('GET', wasmFile)
    request.responseType = 'arraybuffer'
    request.send()
    
    if (my.autoReload) {
        var reloadWhenWasmChanged = function() {
        
            var request = new XMLHttpRequest()
            request.open('HEAD', wasmFile)
            request.setRequestHeader('cache-control', 'no-cache, must-revalidate, post-check=0, pre-check=0');
            request.send()

            request.onload = function() {
                var responseHeaders = request.getAllResponseHeaders().split("\n")
                
                var newChangeToWasm = responseHeaders[1] // FIXME: ugly way of getting the latest changed time
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
