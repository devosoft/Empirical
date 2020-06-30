source ../../third-party/emsdk/emsdk_env.sh
echo "Source loaded"
make
echo "compilation done"
# ./color_map
# for filename in js_utils.js JSWrap.js GetUrlParams.js;
# do
#     node $filename;
#     if [ $? -gt 0 ];
#     then
# 	exit 1;
#     fi;
# done

for test in test_jswrap;
do
    ../../third-party/node_modules/karma/bin/karma start karma.conf.js --filename $test
done
