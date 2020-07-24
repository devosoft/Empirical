#!/usr/bin/env bash

source ../../../third-party/emsdk/emsdk_env.sh
echo "Source loaded"
make all
echo "compilation done"

karmaWebTests=(d3_init scales axis selection transition)   

cd ../../../

# Run Empirical web tests that are compiled javascript and use Karma + Mocha
echo "=== Running compiled JS web tests (via Karma) ==="
for filename in ${karmaWebTests[@]}
do
    third-party/node_modules/karma/bin/karma start tests/web/karma.conf.js --filename d3/${filename}
done

cd tests/web/d3