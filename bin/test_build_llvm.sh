#/home/llvm/bin/clang -cc1 -Ofast -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c11 -fvisibility hidden src/test.c -o dist/test.bc
#/home/llvm/bin/llvm-link -o test.bc dist/*\.bc
#/home/llvm/bin/opt -O3 test.bc -o test.bc
#/home/llvm/bin/llc -O3 -filetype=obj test.bc -o test.o
#/home/llvm/bin/wasm-ld --no-entry test.o -o binary.wasm --strip-all -allow-undefined-file wasm.syms --import-memory

/home/llvm/bin/clang -cc1 -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c11 src/test.c -o dist/test.bc
#/home/llvm/bin/llvm-link -o test.bc dist/*\.bc
#/home/llvm/bin/opt -O3 test.bc -o test.bc
/home/llvm/bin/llc -filetype=obj dist/test.bc -o dist/test.o
/home/llvm/bin/wasm-ld --no-entry dist/test.o -o web/wasm/test_llvm.wasm -allow-undefined-file src/test.js_externals
wasm2wat dist/binary.wasm
