#!/bin/bash

# adapted from https://stackoverflow.com/a/57102498/
if ! [[ "$("${1}" --version)" =~ "Free Software Foundation" ]];
then
  echo "non-GNU ${1} detected"
  exit 1
else
  exit 0
fi
