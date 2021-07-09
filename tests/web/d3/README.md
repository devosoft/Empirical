Compile d3_init.cc test from the `tests/web/d3` directory:
```
emcc -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -I../../../source/ -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 --js-library ../../../source/web/library_emp.js --js-library ../../../source/web/d3/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s WASM=0 d3_init.cc -o d3_init.js
```

Run d3_init.cc test
```
cd ../.. && third-party/node_modules/karma/bin/karma start tests/web/d3/karma.conf.js --filename d3/d3_init
```

If you want to condense these steps you can call:
```
cd d3 && make d3_init.js && .. && ../../third-party/node_modules/karma/bin/karma start karma.conf.js --filename d3/d3_init
```
from the `tests/web` directory.

Or to run all tests at once from the `tests/web/d3` directory:
```
./run_d3_tests.sh
```

## Rolling tests out beyond d3

Refresh the browser?
```
location.reload()
```
No! This breaks testing framework environment (it assumes no refreshes allowed).

Add a container div to wipe every time.