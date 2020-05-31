source ../../third-party/emsdk/emsdk_env.sh
echo "Source loaded"
make
echo "compilation done"
./color_map
for filename in js_utils.js JSWrap.js GetUrlParams.js;
do
    node $filename;
    if [ $? -gt 0 ];
    then
	exit 1;
    fi;
done
