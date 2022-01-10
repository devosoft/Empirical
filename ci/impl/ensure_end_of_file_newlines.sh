#!/bin/bash
set -e

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

# greq -qI tests if file is binary
# adapted from https://stackoverflow.com/a/29689345
TARGETS=$(find . -type f ! -path "./third-party/*" ! -path "./.git/*" ! -path "*/assets/*" ! -path "*.tar.gz" ! -path "*.jpg" ! -path "*.png" ! -path "*Makefile" ! -path "*Maketemplate*" ! -path "./node_modules/*" -exec grep -qI . {} ';' -print)

for filename in ${TARGETS}
do
  sed -i -e '$a\' "${filename}"
done
