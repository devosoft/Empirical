#!/bin/bash
set -e

# adapted from https://stackoverflow.com/a/677212/17332200
if ! command -v "${1}" &> /dev/null;
then
  echo "${1} unavailable"
  exit 1
  # adapted from https://stackoverflow.com/a/57102498/
elif ! [[ "$("${1}" --version)" =~ "Free Software Foundation" ]];
then
  echo "non-GNU ${1} detected"
  exit 1
else
  exit 0
fi
