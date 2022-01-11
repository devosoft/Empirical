#!/bin/bash
set -e

# enforce run as source, not as script
# adapted from https://superuser.com/a/731431
if [[ "$(basename -- "$0")" == "enforce_gnu_version.sh" ]]; then
  echo "$0 may not be run, it must be sourced" >&2
  exit 1
fi

############################################################
# enforcement
############################################################
if ./ci/util/is_gnu_version.sh "${1}";
then
  : # nop
elif ./ci/util/is_gnu_version.sh "g${1}";
then
  echo "using g${1} as ${1}"
  # alias gX as X
  # adapted from https://stackoverflow.com/a/7145401
  eval "function ${1} { \"g${1}\" \"\$@\"; }"
else
  echo "GNU enforcement for ${1} failed"
  exit 1
fi
