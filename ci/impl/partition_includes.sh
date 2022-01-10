#!/bin/bash

TARGETS=$(find . -type f \( -name "*.hpp" -o -name "*.cpp" \) ! -path "./third-party/*" ! -path "./node_modules/*")

for filename in ${TARGETS}
do

  # insert a blank line between #include <'s and #include "'s
  # adapted from https://www.unix.com/shell-programming-and-scripting/186893-how-insert-line-between-two-consecutive-lines-match-special-pattern.html
  awk -v ADD='' -v ANGLEINCLUDE='^#include <.+>$' -v QUOTEINCLUDE='^#include ".+"$' '/^.+$/ L { print L; if(((L ~ ANGLEINCLUDE) && ($0 ~ QUOTEINCLUDE)) || ((L ~ QUOTEINCLUDE) && ($0 ~ ANGLEINCLUDE)) ) print ADD }; { L=$0 } END { print L }' "${filename}" | awk 'NR!=1 {print}' > "${filename}.bak"
  mv "${filename}.bak" "${filename}"

  # insert a blank line between #include "'s and #include "../'s
  # adapted from https://www.unix.com/shell-programming-and-scripting/186893-how-insert-line-between-two-consecutive-lines-match-special-pattern.html
  awk -v ADD='' -v PWDINCLUDE='^#include "[0-9a-zA-Z_].*"$' -v RELATIVEINCLUDE='^#include "../[0-9a-zA-Z_].*"$' '/^.+$/ L { print L; if(((L ~ PWDINCLUDE) && ($0 ~ RELATIVEINCLUDE)) || ((L ~ RELATIVEINCLUDE) && ($0 ~ PWDINCLUDE)) ) print ADD }; { L=$0 } END { print L }' "${filename}" | awk 'NR!=1 {print}' > "${filename}.bak"
  mv "${filename}.bak" "${filename}"

done

echo
