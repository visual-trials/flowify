/home/llvm/bin/clang --target=wasm32 -Os src/test.cpp -nostdlib -c -o dist/test_clang.o
/home/llvm/bin/wasm-ld --no-entry dist/test_clang.o -o web/wasm/test_clang.wasm -allow-undefined-file src/test.js_externals --strip-all --import-memory --export=string_length --export=log232 --export=draw_rectangle --export=render_frame --export=update_frame
# --export=init_world
wasm2wat web/wasm/test_clang.wasm
