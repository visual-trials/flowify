set -e  # stop script on first error

source /home/emscripten/emsdk/emsdk_env.sh

# NOTE: this will not work standalone (due to stack issues!) emcc src/test.c -Os -s WASM=1 -s SIDE_MODULE=1 -o web/wasm/test_emcc.wasm
# So we have to use this commmand, which requires a html or js file (which we don't need atm)
emcc src/test.c -Os -s WASM=1 -o dist/test_emcc.html -s "EXPORTED_FUNCTIONS=['_init_world', '_update_frame', '_render_frame']"
cp dist/test_emcc.wasm web/wasm/test_emcc.wasm
# wasm2wat web/wasm/test_emcc.wasm
