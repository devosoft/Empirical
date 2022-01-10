#!/bin/bash
set -e

# assumes docstring boilerplate is already in place

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

for filename in $(cd include && find -- * -name '*.hpp' -type f); do

  # grow file up to at least 6 lines past close of docstring
  # determined using first blank line or last line
  FILE_NUM_LINES="$(cat "include/${filename}" | wc -l)"
  ONE_PAST_LAST_LINE_NO="$((${FILE_NUM_LINES}+1))"
  # adapted from https://stackoverflow.com/a/66474842
  FIRST_BLANK_LINE_NO="$(awk '! NF { print NR; exit }' "include/${filename}")"
  # two cases: file does or does not have a blank line
  FIRST_EMPTY_LINE_NO="$(grep -q '^$' "include/${filename}" && echo "${FIRST_BLANK_LINE_NO}" || echo "${ONE_PAST_LAST_LINE_NO}")"
  DOCSTRING_CLOSE_LINE_NO="$((${FIRST_EMPTY_LINE_NO}-1))"

  # grow the file, if required
  FILE_NUM_LINES="$(cat "include/${filename}" | wc -l)"
  REQUIRED_NUM_LINES="$((${DOCSTRING_CLOSE_LINE_NO} + 6))"
  for __ in $(seq "${FILE_NUM_LINES}" 1 "${REQUIRED_NUM_LINES}"); do
    echo >> "include/${filename}"
  done

  GUARD=$( sed "s/[^[:alnum:]]/_/g" <<< "$filename" | tr [a-z] [A-Z] )
  NDEF_LINE="#ifndef ${GUARD}_INCLUDE"
  DEF_LINE="#define ${GUARD}_INCLUDE"
  ENDIF_LINE="#endif // ${NDEF_LINE}"

  sed -i "$((${DOCSTRING_CLOSE_LINE_NO}+2))s/^.*\$/${NDEF_LINE}/" "include/${filename}"
  sed -i "$((${DOCSTRING_CLOSE_LINE_NO}+3))s/^.*\$/${DEF_LINE}/" "include/${filename}"
  sed -i "$((${DOCSTRING_CLOSE_LINE_NO}+4))s/^.*\$//" "include/${filename}"
  # ensure blank second-to-last line
  # adapted from https://stackoverflow.com/a/38649101
  sed -i 'x; ${s/.*//;p;x}; 1d' "include/${filename}"
  sed -i '$ s,^.*$,'"${ENDIF_LINE}"',g' "include/${filename}"

done

for filename in $(find * ! -path "third-party/*" ! -path "include/*" -type f -name '*.hpp'); do

  # grow file up to at least 6 lines past close of docstring
  # determined using first blank line or last line
  FILE_NUM_LINES="$(cat "${filename}" | wc -l)"
  ONE_PAST_LAST_LINE_NO="$((${FILE_NUM_LINES}+1))"
  # adapted from https://stackoverflow.com/a/66474842
  FIRST_BLANK_LINE_NO="$(awk '! NF { print NR; exit }' "${filename}")"
  # two cases: file does or does not have a blank line
  FIRST_EMPTY_LINE_NO="$(grep -q '^$' "${filename}" && echo "${FIRST_BLANK_LINE_NO}" || echo "${ONE_PAST_LAST_LINE_NO}")"
  DOCSTRING_CLOSE_LINE_NO="$((${FIRST_EMPTY_LINE_NO}-1))"

  # grow the file, if required
  FILE_NUM_LINES="$(cat "${filename}" | wc -l)"
  REQUIRED_NUM_LINES="$((${DOCSTRING_CLOSE_LINE_NO} + 6))"
  for __ in $(seq "${FILE_NUM_LINES}" 1 "${REQUIRED_NUM_LINES}"); do
    echo >> "${filename}"
  done

  GUARD=$( sed "s/[^[:alnum:]]/_/g" <<< "$filename" | tr [a-z] [A-Z] )
  NDEF_LINE="#ifndef ${GUARD}_INCLUDE"
  DEF_LINE="#define ${GUARD}_INCLUDE"
  ENDIF_LINE="#endif // ${NDEF_LINE}"

  sed -i "$((${DOCSTRING_CLOSE_LINE_NO}+2))s/^.*\$/${NDEF_LINE}/" "${filename}"
  sed -i "$((${DOCSTRING_CLOSE_LINE_NO}+3))s/^.*\$/${DEF_LINE}/" "${filename}"
  sed -i "$((${DOCSTRING_CLOSE_LINE_NO}+4))s/^.*\$//" "${filename}"
  # ensure blank second-to-last line
  # adapted from https://stackoverflow.com/a/38649101
  sed -i 'x; ${s/.*//;p;x}; 1d' "${filename}"
  sed -i '$ s,^.*$,'"${ENDIF_LINE}"',g' "${filename}"

done
