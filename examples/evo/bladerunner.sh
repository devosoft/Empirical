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

confname=${prefix}
confname+=".config"

echo "Making fast-replica..."
make fast-replica > /dev/null

head -n 24 ./replica.cc | tail -n 8 > runs/${confname}

for ((i=0; i<"$2"; i++)); do
    fname=${prefix}
    fname+="$i"
    
    while [[($(jobs -rp | wc -l) > 2)]]; do
        sleep 1
    done
    echo "Launching ${fname}"
    ./replica > runs/${fname} && echo -e "\tDone with ${fname}" &
done

for job in `jobs -p`
do
    wait $job
done
