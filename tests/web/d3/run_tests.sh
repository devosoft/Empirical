source ../../../third-party/emsdk/emsdk_env.sh
echo "Source loaded"
make
echo "compilation done"

karmaWebTests=(d3_init selection transition)                  # Tests that should be run as compiled JS using Karma.

# Run Empirical web tests that are compiled javascript and use Karma + Mocha
echo "=== Running compiled JS web tests (via Karma) ==="
for filename in ${karmaWebTests[@]}
do
    ../../../third-party/node_modules/karma/bin/karma start karma.conf.js --filename ${filename}
done