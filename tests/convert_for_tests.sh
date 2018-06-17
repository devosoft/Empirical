#!/usr/bin/env bash

cp -r ../source ../coverage_source
for filename in $(find ../coverage_source -name "*.h" ! -name "_*" ! -path "*/OLD/*" ! -path "*/in_progress/*" ! -name "*template_instantiations.h")
do
    echo $filename
    ../third-party/force-cover/force_cover $filename -- -I. -I../coverage_source --language c++ -std=c++14 -DEMP_TRACK_MEM -stdlib=libstdc++ | xargs -0 echo > $filename.temp
    mv $filename.temp $filename

done
