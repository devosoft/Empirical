#!/bin/bash
set -e

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

# license notice boilerplate for source files that aren't in include/

for filename in $(find . -name '*.cpp' -type f ! -path "./third-party/*") $(find . -name '*.hpp' -type f ! -path "./third-party/*" ! -path "./include/*"); do

  # grow file up to at least 8 lines
  FILE_NUM_LINES="$(cat "${filename}" | wc -l)"
  for __ in $(seq "${FILE_NUM_LINES}" 1 6); do
    echo >> "${filename}"
  done

  # stamp in expected boilerplate line-by-line
  # just like file docstrings, but don't require a brief
  # stamp in expected boilerplate line-by-line
  sed -i '1s|^.*$|/**|' "${filename}"
  sed -i '2s|^.*$| *  @note This file is part of Empirical, https://github.com/devosoft/Empirical|' "${filename}"
  sed -i '3s|^.*$| *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md|' "${filename}"
  # only match if a @date isn't currently in place
  # if we see @date, "break" (b) sed script for that line
  # adapted from https://stackoverflow.com/a/5334825
  # and https://stackoverflow.com/a/12178023
  # and https://stackoverflow.com/a/9053163
  sed -i "/^ \*  @date /b; 4s/^.*\$/ *  @date $(date +'%Y')/" "${filename}"
  sed -i '5s/^.*$/ */' "${filename}"
  sed -i "6s/^.*\$/ *  @file $(basename "${filename}")/" "${filename}"
  # only match empty lines
  # add extra * to replace later with */ when constructing fresh
  sed -i '7s/^$/ */' "${filename}"

  # close boilerplate file docstring
  # must accomodate possible additional content in docstring
  # so, search backwards from first blank line to place close */
  FILE_NUM_LINES="$(cat "${filename}" | wc -l)"
  ONE_PAST_LAST_LINE_NO="$((${FILE_NUM_LINES}+1))"
  # adapted from https://stackoverflow.com/a/66474842
  FIRST_BLANK_LINE_NO="$(awk '! NF { print NR; exit }' "${filename}")"
  # two cases: file does or does not have a blank line
  FIRST_EMPTY_LINE_NO="$(grep -q '^$' "${filename}" && echo "${FIRST_BLANK_LINE_NO}" || echo "${ONE_PAST_LAST_LINE_NO}")"
  DOCSTRING_CLOSE_LINE_NO="$((${FIRST_EMPTY_LINE_NO}-1))"
  # stamp */ into line preceding first empty line
  sed -i "${DOCSTRING_CLOSE_LINE_NO}s|^.*\$| */|" "${filename}"


done
