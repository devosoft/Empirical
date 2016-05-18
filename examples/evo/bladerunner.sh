#!/bin/bash

# script to automate repeated runs of our simple experiment
# takes in a prefix for the output files and a number of replicants to make

if [[("$#" != 2)]];
then
    echo "** ERROR: Bad number of args."
    echo "** Usage: ./bladerunner run_names num_runs."
    exit 1
fi

prefix=""
prefix+="$1"

for ((i=0; i<"$2"; i++))
do
    fname=${prefix}
    fname+="$i"
    ./replica > runs/${fname}
done
