#!/bin/bash
set -e

# adapted from https://stackoverflow.com/a/835561
# and https://stackoverflow.com/a/4327720
echo "========================================================================="
echo "uncommitted changes detected,"
echo "refusing to perform potentially destructive tidy operation!"
echo
echo "1. commit your changes (including untracked files)"
echo "   -> hint: make clean; git add .; git commit"
echo
echo "or"
echo
echo "2. stash your changes (including untracked files)"
echo "   -> hint: git stash --all"
echo "   -> hint (to recover your stashed changes later): git stash pop"
echo
echo "or"
echo
echo "3. copy your changes elsewhere and remove them here"
echo "   -> hint: cp -r . ../Empirical_bak"
echo "   -> hint: git clean -fxd"
