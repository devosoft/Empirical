#!/usr/bin/env bash

source ../../../third-party/emsdk/emsdk_env.sh
echo "Source loaded"
make all
echo "compilation done"

# emcc -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -I../../../source/ -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 --js-library ../../../source/web/library_emp.js --js-library ../../../source/web/d3/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s WASM=0 d3_init.cc -o d3_init.js
#cd ../../../
#third-party/node_modules/karma/bin/karma start tests/web/karma.conf.js
#cd tests/web/d3

karmaWebTests=(d3_init scales selection transition)   

cd ../../../

# Run Empirical web tests that are compiled javascript and use Karma + Mocha
echo "=== Running compiled JS web tests (via Karma) ==="
for filename in ${karmaWebTests[@]}
do
    third-party/node_modules/karma/bin/karma start tests/web/karma.conf.js --filename d3/${filename}
done

cd tests/web/d3