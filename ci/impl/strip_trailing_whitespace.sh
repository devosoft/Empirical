#!/bin/bash
set -e

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

# greq -qI tests if file is binary
# adapted from https://stackoverflow.com/a/29689345
TARGETS=$(find . -type f ! -path "./third-party/*" ! -path "./.git/*" ! -path "*/assets/*" ! -path "./node_modules/*" -exec grep -qI . {} ';' -print)

for filename in ${TARGETS}
do
# end of files
# adapted from https://unix.stackexchange.com/a/81687
sed -i -e :a -e '/^\n*$/{$d;N;};/\n$/ba' "${filename}"

# end of lines
sed -i 's/\s*$//g' "${filename}"
done
