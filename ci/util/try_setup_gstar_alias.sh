#!/bin/bash
set -e

# enforce run as source, not as script
# adapted from https://superuser.com/a/731431
if [[ "$(basename -- "$0")" == "try_setup_gstar_alias.sh" ]]; then
  echo "$0 may not be run, it must be sourced" >&2
  exit 1
fi

# is g* alias available?
# adapted from https://stackoverflow.com/a/677212/17332200
if command -v "g${1}" &> /dev/null;
then
  # actually make the alias
  echo "using g${1} as ${1}"
  # adapted from https://stackoverflow.com/a/7145401
  eval "function "${1}" { \"g${1}\" \"\$@\"; }"
  exit 0
else
  echo "g${1} alias not available"
  exit 1
fi
