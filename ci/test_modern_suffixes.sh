#!/bin/bash

# refuse to continue if uncommitted changes are present
# adapted from https://stackoverflow.com/a/40535565
if ! [[ -z $(git status -s) ]];
then
  ./ci/print_uncommitted_changes_warning.sh
  exit 1
fi

SOURCE_HASH=$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )

./ci/modernize_filename_suffixes.sh

if [ "${SOURCE_HASH}" == "$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )" ];
then
  exit 0 # success
else
  echo "old-style suffixes in filenames detected, run ./ci/modernize_filename_suffixes.sh locally to find & fix"
  exit 1 # failure
fi