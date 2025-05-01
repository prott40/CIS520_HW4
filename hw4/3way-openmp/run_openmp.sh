#!/bin/bash
#SBATCH --mem-per-cpu=512MB
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --constraint=moles

# Thread configurations
THREADS=("1" "2" "4" "8" "16" "20")

# Versions and corresponding source files
VERSIONS=("V1" "V2" "V3")

# Clean up previous run results
rm -rf previous_results
if [ -d results ]; then
    mv results previous_results
fi
mkdir results

# Compile each version of the program
for VERSION in "${VERSIONS[@]}"; do
    gcc -fopenmp -O2 -g -march=native -o max_char$VERSION openmpmech$VERSION.c
done

# Run each version with each thread count
for VERSION in "${VERSIONS[@]}"; do
    for THREAD_COUNT in "${THREADS[@]}"; do
        # Create directory for each version/thread combination
        DIR="results/$VERSION/threads_$THREAD_COUNT"
        mkdir -p "$DIR"

        # Set OpenMP environment
        export OMP_NUM_THREADS=$THREAD_COUNT
        export OMP_STACKSIZE=64M

        # Navigate to the results directory
        cd "$DIR"
        touch perf_stat_summary.txt

        # Run perf stat summary
        perf stat -e task-clock,cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
            -o perf_stat_summary.txt \
            ../../../max_char$VERSION

        # Calculate and append estimated CPU utilization
        UTIL=$(grep 'task-clock' perf_stat_summary.txt | awk '{print $1}' | tr -d ,)
        ELAPSED=$(grep 'seconds time elapsed' perf_stat_summary.txt | awk '{print $1}')
        CPU_UTIL=$(awk -v t="$UTIL" -v e="$ELAPSED" -v n="$THREAD_COUNT" 'BEGIN { printf("%.2f%%", 100*t/(e*1000*n)) }')
        echo "Estimated CPU utilization: $CPU_UTIL" >> perf_stat_summary.txt

        # Run interval-based perf stat timeline
        perf stat -e task-clock,cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
            -I 100 -x, -o perf_stat_timeline.csv \
            ../../../max_char$VERSION

        echo "Performance summary for $VERSION with $THREAD_COUNT threads saved to $DIR"

        # Return to base directory
        cd ../../../
    done
done