#!/bin/bash

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

find ! -path "./third-party/*" ! -path "./node_modules/*" -type d | grep '\s' | rename 's/\s/_/g'    # do the directories first
find ! -path "./third-party/*" ! -path "./node_modules/*" -type f | grep '\s' | rename 's/\s/_/g'    # do the directories first
