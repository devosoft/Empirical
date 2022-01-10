#!/bin/bash

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
elif . ./ci/util/try_setup_gstar_alias.sh "${1}";
then
  . ./ci/util/enforce_gnu_version.sh
else
  echo "GNU enforcement for ${1} failed"
  exit 1
fi
