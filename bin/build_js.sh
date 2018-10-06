set -e  # stop script on first error

source /home/emscripten/emsdk/emsdk_env.sh

# NOTE: this will not work standalone (due to stack issues!) emcc src/flowify.cpp -Os -s WASM=1 -s SIDE_MODULE=1 -o web/wasm/flowify.wasm
emcc src/flowify.cpp -Os -s WASM=1 -o dist/flowify.html --js-library web/js/emcc/external.js -s "EXPORTED_FUNCTIONS=['_init_world', '_update_frame', '_render_frame', '_set_left_mouse_button_data', '_set_right_mouse_button_data', '_set_mouse_wheel_data', '_set_mouse_position_data']"
cp dist/flowify.wasm web/wasm/flowify.wasm
# wasm2wat web/wasm/flowify.wasm

