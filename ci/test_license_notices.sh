#!/bin/bash
set -e

# enforce use of GNU version of coreutils
. ./ci/util/enforce_gnu_utils.sh

# refuse to continue if uncommitted changes are present
. ./ci/util/enforce_git_status.sh

SOURCE_HASH=$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )

./ci/impl/generate_license_notices.sh

if [ "${SOURCE_HASH}" == "$( find -path ./third-party -prune -false -o -type f | sort | xargs cat | sha1sum )" ];
then
  exit 0 # success
else
  echo "malformed or missing license notices detected, run ./ci/impl/generate_license_notices.sh locally to find & fix"
  exit 1 # failure
fi
