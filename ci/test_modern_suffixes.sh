#!/bin/bash

SOURCE_HASH=$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )

./ci/modernize_filename_suffixes.sh

if [ "${SOURCE_HASH}" == "$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )" ];
then
  exit 0 # success
else
  echo "old-style suffixes in filenames detected, run ./ci/modernize_filename_suffixes.sh locally to find & fix"
  exit 1 # failure
fi
