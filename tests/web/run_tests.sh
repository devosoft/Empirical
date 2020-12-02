#!/bin/bash

source ../../third-party/emsdk/emsdk_env.sh
echo "Source loaded"
make
echo "compilation done"

# Bank of tests to run.
catchWebTests=(color_map)               # Tests that should be run as compiled C++ executables (that ideally use Catch like all other non-web tests).
nodeWebTests=(GetUrlParams)    # Tests that should be run as compiled JS scripts via node.
karmaWebTests=(jswrap widget js_utils element test_visualizations)                  # Tests that should be run as compiled JS using Karma.

# Run Empirical web tests that are compiled C++ programs that (SHOULD) use the Catch framework.
echo "=== Running compiled C++ web tests ==="
for filename in ${catchWebTests[@]}
do
    ./${filename}.out
done

# Run Empirical web tests that should be run as compiled javascript and should run via node.
# i.e., do not need to run w/karma because they don't need a browser and/or don't use emp::web::TestManager
#       or the Mocha test framework
echo "=== Running compiled JS web tests (via node) ==="
cp ../../third-party/package.json .
npm install
for filename in ${nodeWebTests[@]}
do
    node ${filename}.js;
    if [ $? -gt 0 ];
    then
	exit 1;
    fi;
done

# Run Empirical web tests that are compiled javascript and use Karma + Mocha
echo "=== Running compiled JS web tests (via Karma) ==="
for filename in ${karmaWebTests[@]}
do
    ../../third-party/node_modules/karma/bin/karma start karma.conf.js --filename ${filename}
done
