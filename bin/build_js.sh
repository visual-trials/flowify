# First run emsdk_env.sh inside C:\emscripten\emsdk

emcc src/flowify.cpp -Os -s WASM=1 -s SIDE_MODULE=1 -o web/wasm/flowify.wasm


