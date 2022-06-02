#!/bin/bash
set -e

# enforce run as source, not as script
# adapted from https://superuser.com/a/731431
if [[ "$(basename -- "$0")" == "enforce_git_status.sh" ]]; then
  echo "$0 may not be run, it must be sourced" >&2
  exit 1
fi

# refuse to continue if uncommitted changes are present
# adapted from https://stackoverflow.com/a/40535565
if ! [[ -z $(git status -s) ]];
then
  ./ci/util/print_uncommitted_changes_warning.sh
  exit 1
fi

# refuse to continue if gitignored files are present
# adapted from https://stackoverflow.com/a/467053
if ! [[ -z $(git check-ignore $(\
  find -path ./third-party -prune -false -o -type f)) ]];
then
  ./ci/util/print_gitignored_files_warning.sh
  exit 1
fi
