#!/bin/bash

SOURCE_HASH=$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )

./ci/ensure_end_of_file_newlines.sh

if [ "${SOURCE_HASH}" == "$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )" ];
then
  exit 0 # success
else
  echo "missing end of file newlines detected, run ./ci/ensure_end_of_file_newlines.sh locally to find & fix"
  echo "why? see https://stackoverflow.com/a/729795"
  exit 1 # failure
fi
