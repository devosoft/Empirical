#!/bin/bash

SOURCE_HASH=$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )

./ci/strip_filename_whitespace.sh

if [ "${SOURCE_HASH}" == "$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )" ];
then
  exit 0 # success
else
  echo "whitespace in filenames detected, run ./ci/strip_filename_whitespace.sh locally to find & fix"
  exit 1 # failure
fi
