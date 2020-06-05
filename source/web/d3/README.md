# D3 JS Empirical Wrapper

Version of d3: v5.16.0

To compile (from the d3 directory)
```
em++ -std=c++17 -I../../ -Os --js-library ../library_emp.js --js-library library_d3.js  -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'stringToUTF8']" -s NO_EXIT_RUNTIME=1 test.cc -o main.js
```