#!/bin/bash
set -e

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

# enforce availability of dependencies
. ./ci/util/enforce_dependency.sh rename

find ! -path "./third-party/*" ! -path "./.git/*" -type f | rename 's/\.h$/.hpp/'
find ! -path "./third-party/*" ! -path "./.git/*" -type f | rename 's/\.c$/.cpp/'
find ! -path "./third-party/*" ! -path "./.git/*" -type f | rename 's/\.cc$/.cpp/'
