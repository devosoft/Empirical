# D3 JS Empirical Wrapper

Version of d3: v5.16.0

To compile (from the d3 directory)
```
em++ -std=c++17 -I../../ -Os --js-library ../library_emp.js --js-library library_d3.js  -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'stringToUTF8']" -s NO_EXIT_RUNTIME=1 test.cc -o main.js
```

If that doesn't work because of the mysterious and evil `Fatal: Unexpected arg0 type (select) in call to: emscripten_asm_const_int` error, try this command:
```
emcc -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -I../../  -pedantic -Wno-dollar-in-identifier-extension -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s TOTAL_MEMORY=67108864 --js-library ../library_emp.js --js-library library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s WASM=0  test.cc -o main.js
```