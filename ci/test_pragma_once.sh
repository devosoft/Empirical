#!/bin/bash
set -e

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

# adapted from https://www.linsoftware.com/how-to-check-for-git-merge-conflicts-in-travis-ci/
found_pragma_onces=$(grep -EHlr --exclude=test_pragma_once.sh --exclude=getting-started.md --exclude-dir=third-party '#pragma once' . || :)

if [ -z "$found_pragma_onces" ]
then
  exit 0 # success
else
  echo "pragma onces found... use header guards instead!"
  echo $found_pragma_onces
  exit 1 # failure
fi
