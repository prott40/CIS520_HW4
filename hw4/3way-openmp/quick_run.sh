#!/bin/bash
#SBATCH --mem-per-cpu=512MB
#SBATCH --cpus-per-task=16
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --constraint=moles

THREADS=("1" "2" "4" "8" "16")
LINE_COUNT="1000000"
VERSION="V1"

rm -rf previous_quick_results
if [ -d quick_results ]; then
    mv quick_results previous_quick_results
fi
mkdir quick_results

# Compile only V1
gcc -fopenmp -O2 -g -march=native -o max_char$VERSION openmpmech$VERSION.c

# Main loop
for THREAD_COUNT in "${THREADS[@]}"; do
    DIR="quick_results/$VERSION/threads_${THREAD_COUNT}"
    mkdir -p "$DIR"

    export OMP_NUM_THREADS=$THREAD_COUNT
    export OMP_STACKSIZE=64M

    cd "$DIR"
    touch perf_stat_summary.txt

    # Run with perf + time
    /usr/bin/time -v perf stat -e task-clock,cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
    -o perf_stat_tmp.txt \
    ../../../max_char$VERSION $LINE_COUNT > program_output.txt 2> time_output.txt

    # Combine outputs
    cat perf_stat_tmp.txt >> perf_stat_summary.txt
    echo "" >> perf_stat_summary.txt
    grep "Maximum resident set size" time_output.txt >> perf_stat_summary.txt

    rm perf_stat_tmp.txt time_output.txt

    # Estimate CPU utilization
    UTIL=$(grep 'task-clock' perf_stat_summary.txt | awk '{print $1}' | tr -d ,)
    ELAPSED=$(grep 'seconds time elapsed' perf_stat_summary.txt | awk '{print $1}')
    CPU_UTIL=$(awk -v t="$UTIL" -v e="$ELAPSED" -v n="$THREAD_COUNT" 'BEGIN { printf("%.2f%%", 100*t/(e*1000*n)) }')
    echo "Estimated CPU utilization: $CPU_UTIL" >> perf_stat_summary.txt

    echo "Test: $VERSION, $LINE_COUNT lines, $THREAD_COUNT threads saved to $DIR"
    cd ../../../
done
