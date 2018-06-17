#!/usr/bin/env bash

set -e # exit with error if any of this fails
cp -r ../source ../coverage_source
for filename in $(find ../coverage_source -name "*.h" ! -name "_*" ! -path "*/OLD/*" ! -path "*/in_progress/*" ! -path "*/web/*")
do
    echo $filename
    ../third-party/force-cover/force_cover $filename -- -I. -I../coverage_source --language c++ -std=c++14 -DEMP_TRACK_MEM -I/usr/bin/../lib/gcc/x86_64-linux-gnu/5.5.0/../../../../include/c++/5.5.0 -I/usr/bin/../lib/gcc/x86_64-linux-gnu/5.5.0/../../../../include/x86_64-linux-gnu/c++/5.5.0 -I/usr/bin/../lib/gcc/x86_64-linux-gnu/5.5.0/../../../../include/c++/5.5.0/backward -I/usr/include/clang/5.0.2/include -I/usr/local/include -I/usr/bin/../lib/gcc/x86_64-linux-gnu/5.5.0/include -I/usr/include/x86_64-linux-gnu -I/usr/include | xargs -0 echo > $filename.temp
    mv $filename.temp $filename

done
