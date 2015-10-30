#!/bin/bash
INVENV=-1
python -c 'import sys; print sys.real_prefix' 2>/dev/null && INVENV=1 || INVENV=0

if [ "$INVENV" -eq 0 ]; then
	echo "WARNING! You are NOT in a virtual environment!"
	read -p "Are you sure you wish to continue (y/n)? " -n 1 -r
	echo    # (optional) move to a new line
	if [[ $REPLY =~ ^[Yy]$ ]]
	then
		pip install sphinx 
	else
		exit 0
	fi
else
	pip install sphinx
fi

