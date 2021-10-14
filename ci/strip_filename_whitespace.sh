#!/bin/bash

find ! -path "./third-party/*" ! -path "./node_modules/*" -type d | grep '\s' | rename 's/\s/_/g'    # do the directories first
find ! -path "./third-party/*" ! -path "./node_modules/*" -type f | grep '\s' | rename 's/\s/_/g'    # do the directories first
