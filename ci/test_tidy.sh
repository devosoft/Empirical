#!/bin/bash

set -e

echo "Running tidyness enforcement tests..."
echo "See https://empirical.readthedocs.io/en/latest/dev/guide-to-testing.html#tidyness-enforcement for info on tidyness enforcement."
echo "(Including how to automatically generate tidyness fixes if tidyness enforcement detects tidyness issues.)"

./ci/test_alphabetize_includes.sh && echo "✔ include alphabetization ok" || exit 1
./ci/test_alphabetize_imports.sh && echo "✔ import alphabetization ok" || exit 1
./ci/test_partition_includes.sh && echo "✔ include partitioning ok" || exit 1
./ci/test_boilerplate.sh && echo "✔ boilerplate ok" || exit 1
./ci/test_license_notices.sh && echo "✔ license notices ok" || exit 1
./ci/test_merge_conflict_markers.sh && echo "✔ conflict markers ok" || exit 1
./ci/test_end_of_file_newlines.sh && echo "✔ EOF newlines ok" || exit 1
./ci/test_tabs.sh && echo "✔ no tabs, indentation ok" || exit 1
./ci/test_2space_indentation.sh && echo "✔ 2-space indents, indentation ok" || exit 1
./ci/test_trailing_whitespace.sh && echo "✔ trailing whitespace ok" || exit 1
./ci/test_filename_whitespace.sh && echo "✔ filename whitespace ok" || exit 1
./ci/test_modern_suffixes.sh && echo "✔ modern suffixes ok" || exit 1
./ci/test_make_clean.sh && echo "✔ no compilation artifacts" || exit 1
