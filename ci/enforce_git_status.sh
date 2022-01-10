#!/bin/bash

# adapted from https://superuser.com/a/731431
if [[ "$(basename -- "$0")" == "script.sh" ]]; then
  echo "$0 may not be run, it must be sourced" >&2
  exit 1
fi

# refuse to continue if uncommitted changes are present
# adapted from https://stackoverflow.com/a/40535565
if ! [[ -z $(git status -s) ]];
then
  ./ci/print_uncommitted_changes_warning.sh
  exit 1
fi
