#!/bin/bash

SOURCE_HASH=$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )

./ci/alphabetize_imports.sh

if [ "${SOURCE_HASH}" == "$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )" ];
then
  exit 0 # success
else
  echo "unalphabetized imports detected, run ./ci/alphabetize_imports.sh locally to find & fix"
  exit 1 # failure
fi
