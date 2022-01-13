#!/bin/bash
set -e

# adapted from https://apple.stackexchange.com/a/88812
echo "========================================================================="
echo "GNU coreutils unavailable, refusing to continue"
echo
echo "on macOS, use homebrew <https://brew.sh/>"
echo "to easily install GNU coreutils to their g* aliases"
echo "(ex. find -> gfind, etc.)"
echo
echo "brew install coreutils"
echo "brew install findutils"
echo "brew install gnu-sed"
echo "brew install gawk"
echo "brew install grep"
echo
echo "if you use MacPorts, run"
echo
echo "sudo port install coreutils"
echo "sudo port install findutils"
echo "sudo port install gsed"
echo "sudo port install gawk"
echo "sudo port install grep"
echo
echo "other BSD users will also need to find a way"
echo "to install GNU coreutils to their g* aliases"
