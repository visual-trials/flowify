source /home/emscripten/emsdk/emsdk_env.sh

emcc src/flowify.cpp -Os -s WASM=1 -s SIDE_MODULE=1 -o web/wasm/flowify.wasm
