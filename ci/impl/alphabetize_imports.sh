#!/bin/bash

TARGETS=$(find . -type f \( -name "*.py" -o -name "*.ipynb" \) ! -path "./third-party/*")

for filename in ${TARGETS}
do

  # skip empty files
  # adapted from https://stackoverflow.com/a/27710284
  [ -s "${filename}" ] || continue

  printf "."

  # adapted from https://stackoverflow.com/a/6970681
  include_linenos="$(awk '/^import .*| *"import .*|^from .* import .*| *"from .* import .*/ {print FNR}' "${filename}")"

  # adapted from https://stackoverflow.com/a/26809816
  # and https://superuser.com/a/284192
  echo "${include_linenos}" | tr ' ' '\n' | awk 'NR==1{first=$1;last=$1;next} $1 == last+1 {last=$1;next} {print first,last;first=$1;last=first} END{print first,last}' | while read line ; do

    # adapted from https://unix.stackexchange.com/a/11064
    read start stop <<< "${line}"

    # adapted from https://stackoverflow.com/a/46018238
    # and https://stackoverflow.com/a/32723119
    # and https://unix.stackexchange.com/a/87748
    export LC_ALL=en_US.UTF-8
    echo "x" | ex -s -c "${start},${stop}!sort --stable --ignore-nonprinting --ignore-case --dictionary-order --key=2" "${filename}"
    # see also https://www.gnu.org/software/coreutils/manual/html_node/sort-invocation.html

  done
done

echo
