#!/bin/bash
#INVENV=-1
#python -c 'import sys; sys.exit(hasattr(sys, "real_prefix"))' && INVENV=1 || INVENV=0
#python -c 'import sys; sys.exit(hasattr(sys, "real_prefix"))'
#INVENV=$?
#if [ "$INVENV" -eq 0 ]; then
#	echo "WARNING! You are NOT in a virtual environment!"
#	read -p "Are you sure you wish to continue (y/n)? " -n 1 -r
#	echo    # (optional) move to a new line
#	if [[ $REPLY =~ ^[Yy]$ ]]
#	then
#		env/bin/pip install -r requirements.txt
#	else
#		exit 0
#	fi
#else
#	pip install -r requirements.txt
#fi
env/bin/pip install -r requirements.txt
source env/bin/activate
