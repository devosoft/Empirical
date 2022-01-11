#!/bin/bash
set -e

# enforce run as source, not as script
# adapted from https://superuser.com/a/731431
if [[ "$(basename -- "$0")" == "enforce_dependency.sh" ]]; then
  echo "$0 may not be run, it must be sourced" >&2
  exit 1
fi

# is the dependency unavailable?
# adapted from https://stackoverflow.com/a/677212/
if ! command -v "${1}" &> /dev/null;
then
  echo "======================================================================="
  echo "${1} unavailable, refusing to continue"
  echo
  echo "use your local package manager to install it"
  echo "i.e., brew on macOS, apt or yum on Linux, etc."
fi
