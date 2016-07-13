#!/bin/bash
# launch the specified number of threads

for ((i=0; i < "$2"; i++)); do
    
    while [[( $(jobs -rp | wc -l) > 6 ) ]]; do
        sleep 1
    done
    echo "Launching ${i}"
    ./gladiator-standard.out -PREFIX defector-${i} -RAND_SEED ${i} -LO_AI_WEIGHT 0.8 -HI_AI_WEIGHT 0.3 \
    -INITIAL_CONFIG 4 -TICKS 80000 -AVAILABLE_PRIVATE_PTS 75000 -PERCENT_OTHER 0 \
    -ENABLE_MUTATION 1 -BOTTLENECK_LETHALITY 0.90 -BOTTLENECK_SPACING 300 -MUTATION_AMOUNT 0.01 \
    -PERCENT_STARTING .1 2>/dev/null && echo -e "\tDone with ${i}" &
done

# wait for threads to join
for job in `jobs -p`
do
    wait $job
done
