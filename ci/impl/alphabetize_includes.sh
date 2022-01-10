#!/bin/bash
set -e

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

TARGETS=$(find . -type f \( -name "*.hpp" -o -name "*.cpp" \) ! -path "./third-party/*" ! -path "./node_modules/*")

for filename in ${TARGETS}
do

  printf "."

  # adapted from https://stackoverflow.com/a/6970681
  include_linenos=$(awk '/^#include .*/ {print FNR}' "${filename}")

  # adapted from https://stackoverflow.com/a/26809816
  # and https://superuser.com/a/284192
  echo "${include_linenos}" | tr ' ' '\n' | awk 'NR==1{first=$1;last=$1;next} $1 == last+1 {last=$1;next} {print first,last;first=$1;last=first} END{print first,last}' | while read line ; do

    # adapted from https://unix.stackexchange.com/a/11064
    read start stop <<< "${line}"

    # adapted from https://stackoverflow.com/a/46018238
    # and https://stackoverflow.com/a/32723119
    # and https://unix.stackexchange.com/a/87748
    export LC_ALL=C
    echo "x" | ex -s -c "${start},${stop}!sort --stable --ignore-nonprinting --ignore-case --dictionary-order" $filename
    # see also https://www.gnu.org/software/coreutils/manual/html_node/sort-invocation.html

  done
done

echo
