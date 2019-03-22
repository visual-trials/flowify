set -e  # stop script on first error

source /home/emscripten/emsdk/emsdk_env.sh

#PROJECT_NAME=flowify
#PROJECT_NAME=testing/testing_file
PROJECT_NAME=testing/testing_parser
#PROJECT_NAME=testing/testing_memory
#PROJECT_NAME=testing/testing_text
#PROJECT_NAME=testing/testing_animation
#PROJECT_NAME=testing/testing_keyboard
#PROJECT_NAME=testing/testing_touch
#PROJECT_NAME=testing/testing_mouse
#PROJECT_NAME=testing/testing_renderer

# Debug build: DO_ASSERTIONS=1

emcc src/platform/browser_main.cpp -Os -s WASM=1 -D DO_ASSERTIONS=1 -D INCLUDE_PROJECT_FILE="\"../$PROJECT_NAME.c\"" -o dist/$PROJECT_NAME.html -s TOTAL_MEMORY=34MB --js-library web/js/emcc/external.js \
    -s "EXPORTED_FUNCTIONS=[
        '_init_world', 
        '_update_frame', 
        '_render_frame', 
        '_set_left_mouse_button_data', 
        '_set_right_mouse_button_data', 
        '_set_mouse_wheel_data', 
        '_set_mouse_position_data', 
        '_get_address_keys_that_are_down', 
        '_get_address_sequence_keys_up_down', 
        '_set_sequence_keys_length', 
        '_set_touch_count', 
        '_set_touch_data', 
        '_get_address_file_upload',
        '_get_address_file_name',
        '_set_file_was_uploaded',
        '_set_frame_time',
        '_set_screen_size',
        '_set_address_and_size_dynamic_memory'
        ]"
cp dist/$PROJECT_NAME.wasm web/wasm/$PROJECT_NAME.wasm
# wasm2wat web/wasm/$PROJECT_NAME.wasm

# NOTE: this will not work standalone (due to stack issues!) emcc src/platform/browser_main.cpp -Os -s WASM=1 -s SIDE_MODULE=1 -o web/wasm/$PROJECT_NAME.wasm
