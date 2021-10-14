#!/bin/bash

TARGETS=$(find . -type f \( -name "*.hpp" -o -name "*.cpp" \) ! -path "./third-party/*" ! -path "./node_modules/*")

for filename in ${TARGETS}; do

  if grep -qE '^   ' "${filename}" && ! grep -qE '^  \S' "${filename}"; then
    echo "greater than 2-space indentation found in ${filename}"
    echo "(found 3+-space indentation without any exactly 2-space indents)"
    exit 1 # failure
  fi

done

exit 0 # success
