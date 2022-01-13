#!/bin/bash
set -e

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

for filename in $(cd include && find -- * -name '*.hpp' -type f); do

  # grow file up to at least 9 lines
  FILE_NUM_LINES="$(cat "include/${filename}" | wc -l)"
  for __ in $(seq "${FILE_NUM_LINES}" 1 7); do
    echo >> "include/${filename}"
  done

  # stamp in expected boilerplate line-by-line
  sed -i '1s|^.*$|/**|' "include/${filename}"
  sed -i '2s|^.*$| *  @note This file is part of Empirical, https://github.com/devosoft/Empirical|' "include/${filename}"
  sed -i '3s|^.*$| *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md|' "include/${filename}"
  # only match if a @date isn't currently in place
  # if we see @date, "break" (b) sed script for that line
  # adapted from https://stackoverflow.com/a/5334825
  # and https://stackoverflow.com/a/12178023
  # and https://stackoverflow.com/a/9053163
  sed -i "/^ \*  @date /b; 4s/^.*\$/ *  @date $(date +'%Y')/" "include/${filename}"
  sed -i '5s/^.*$/ */' "include/${filename}"
  sed -i "6s/^.*\$/ *  @file $(basename "${filename}")/" "include/${filename}"
  # only match if a @brief isn't currently in place
  # adapted from https://stackoverflow.com/a/5334825
  sed -i "/^ \*  @brief /b; 7s/^.*\$/ *  @brief TODO./" "include/${filename}"
  # only match empty lines
  # add extra * to replace later with */ when constructing fresh
  sed -i '8s/^$/ */' "include/${filename}"

  # close boilerplate file docstring
  # must accomodate possible additional content in docstring
  # so, search backwards from first blank line to place close */
  FILE_NUM_LINES="$(cat "include/${filename}" | wc -l)"
  ONE_PAST_LAST_LINE_NO="$((${FILE_NUM_LINES}+1))"
  # adapted from https://stackoverflow.com/a/66474842
  FIRST_BLANK_LINE_NO="$(awk '! NF { print NR; exit }' "include/${filename}")"
  # two cases: file does or does not have a blank line
  FIRST_EMPTY_LINE_NO="$(grep -q '^$' "include/${filename}" && echo "${FIRST_BLANK_LINE_NO}" || echo "${ONE_PAST_LAST_LINE_NO}")"
  DOCSTRING_CLOSE_LINE_NO="$((${FIRST_EMPTY_LINE_NO}-1))"
  # stamp */ into line preceding first empty line
  sed -i "${DOCSTRING_CLOSE_LINE_NO}s|^.*\$| */|" "include/${filename}"

done
