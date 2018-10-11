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
        
        input.sendMouseData()
        input.sendTouchData()
        input.sendKeyboardData()
        
        // Update world
        my.wasmInstance.exports._update_frame()
        
        // Render world
        my.wasmInstance.exports._render_frame()

        input.resetMouseData()
        input.resetTouchData()
        input.resetKeyboardData()

        Flowify.canvas.requestAnimFrame(my.mainLoop)
    }
    
    let wasmEnv = {}
    
    // This calculation of STACKTOP, DYNAMICTOP_PTR and DYNAMIC_BASE is extracted from the .js file emcc generated
    {
        function alignMemory(size, factor) {
            if (!factor) factor = STACK_ALIGN
            var ret = size = Math.ceil(size / factor) * factor
            return ret
        }
        
        function staticAlloc(size) {
            let ret = STATICTOP
            STATICTOP = STATICTOP + size + 15 & -16
            return ret
        }

        let WASM_PAGE_SIZE = 65536

        let STACK_ALIGN = 16
        let STATIC_BASE, STATICTOP, staticSealed
        let STACK_BASE, STACKTOP, STACK_MAX
        let DYNAMIC_BASE, DYNAMICTOP_PTR
        
        STATIC_BASE = STATICTOP = STACK_BASE = STACKTOP = STACK_MAX = DYNAMIC_BASE = DYNAMICTOP_PTR = 0
        
        let TOTAL_STACK =   5242880 // Module["TOTAL_STACK"]
        let TOTAL_MEMORY = 33554432 // Module["TOTAL_MEMORY"]

        let GLOBAL_BASE = 1024
        STATIC_BASE = GLOBAL_BASE
        STATICTOP = STATIC_BASE + 16992
        let STATIC_BUMP = 16992
        STATICTOP += 16
        
        let memory = new WebAssembly.Memory({
                initial: TOTAL_MEMORY / WASM_PAGE_SIZE,  // 512 = 32MB
                maximum: TOTAL_MEMORY / WASM_PAGE_SIZE,
        })
        
        let table = new WebAssembly.Table({
                initial: 1024,
                maximum: 1024,
                element: 'anyfunc',
        })
        
        DYNAMICTOP_PTR = staticAlloc(4)
        STACK_BASE = STACKTOP = alignMemory(STATICTOP)
        STACK_MAX = STACK_BASE + TOTAL_STACK
        DYNAMIC_BASE = alignMemory(STACK_MAX)
        
        my.bufferU8 = new Uint8Array(memory.buffer)
        my.bufferI32 = new Int32Array(memory.buffer)

        my.bufferI32[DYNAMICTOP_PTR >> 2] = DYNAMIC_BASE
        
        wasmEnv = {
            STACKTOP: STACKTOP,
            memoryBase: STATIC_BASE,
            tableBase: 0,
            memory: memory,
            table: table,
        }
        
        wasmEnv._emscripten_memcpy_big = function(dest, src, num) {
            my.bufferU8.set(my.bufferU8.subarray(src, src+num), dest)
            return dest
        }
    }
    
    let exportedFunctions = Flowify.canvas.getExportedFunctions()
    for (let functionName in exportedFunctions) {
        wasmEnv[functionName] = exportedFunctions[functionName]
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
            
            Flowify.input.addressKeysThatAreDown = my.wasmInstance.exports._get_address_keys_that_are_down()
            Flowify.input.addressSequenceKeysUpDown = my.wasmInstance.exports._get_address_sequence_keys_up_down()

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
