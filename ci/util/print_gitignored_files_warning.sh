#!/bin/bash
set -e

# adapted from https://stackoverflow.com/a/835561
# and https://stackoverflow.com/a/4327720
# and https://git-scm.com/book/en/v2/Git-Tools-Stashing-and-Cleaning
echo "========================================================================="
echo "gitignored files detected,"
echo "refusing to perform potentially destructive tidy operation!"
echo
echo "1. stash your changes (including ignored files)"
echo "   -> hint: git stash --all"
echo "   -> hint (to recover your stashed changes later): git stash pop"
echo
echo "or"
echo
echo "2. copy your gitignored files elsewhere and remove them here"
echo "   -> hint: cp -r . ../Empirical_bak"
echo "   -> hint: git clean -fxd"
echo
echo "or"
echo
echo "3. list gitignored files and then, if ok, just delete them"
echo "   -> hint: git check-ignore $(find -path ./third-party -prune -false -o -type f)"
echo "   -> hint: rm -f $(git check-ignore $(find -path ./third-party -prune -false -o -type f))"
