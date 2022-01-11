#!/bin/bash
set -e

# enforce run as script, not as source
# adapted from https://superuser.com/a/731431
if ! [[ "$(basename -- "$0")" == "is_gnu_version.sh" ]]; then
  echo "$0 may not be sourced, it must be run" >&2
  exit 1
fi


# adapted from https://stackoverflow.com/a/677212/
if ! command -v "${1}" &> /dev/null;
then
  echo "${1} unavailable"
  exit 1
elif ! "${1}" --version &>/dev/null;
then
  echo "${1} version info unavailable"
  exit 1
# adapted from https://stackoverflow.com/a/57102498/
elif ! [[ "$("${1}" --version)" =~ "Free Software Foundation" ]];
then
  echo "non-GNU ${1} detected"
  exit 1
else
  exit 0
fi
