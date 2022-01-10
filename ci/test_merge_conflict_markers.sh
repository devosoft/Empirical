#!/bin/bash
set -e

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

# adapted from https://www.linsoftware.com/how-to-check-for-git-merge-conflicts-in-travis-ci/

found_merge_markers=$(grep -EHlr --exclude=test_merge_conflict_markers.sh --exclude=getting-started.md --exclude-dir=third-party '<<<<<<< HEAD|>>>>>>>' . || :)

if [ -z "$found_merge_markers" ]
then
  exit 0 # success
else
  echo "git merge conflict markers found"
  echo $found_merge_markers
  exit 1 # failure
fi
