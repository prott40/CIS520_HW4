#!/bin/bash
#SBATCH --mem-per-cpu=512MB
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --constraint=moles

# Define the thread configurations
THREADS=("1" "2" "4" "8" "16" "20")

# Clean up previous run results
rm -rf previous_results
if [ -d results ]; then
    mv results previous_results
fi
mkdir results

# Loop over each thread count
for THREAD_COUNT in "${THREADS[@]}"; do
    # Create a directory for each thread count
    mkdir results/threads_$THREAD_COUNT

    # Set OpenMP environment
    export OMP_NUM_THREADS=$THREAD_COUNT
    export OMP_STACKSIZE=64M

    # Compile OpenMP code
    gcc -fopenmp -O2 -g -march=native -o max_char openmpmech.c

    # Navigate to the results directory
    cd results/threads_$THREAD_COUNT

    touch perf_stat_summary.txt

    # Run basic perf stat (one-line summary with task-clock)
    perf stat -e task-clock,cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
        -o perf_stat_summary.txt \
        ../../max_char

    # Calculate and append estimated CPU utilization
    UTIL=$(grep 'task-clock' perf_stat_summary.txt | awk '{print $1}' | tr -d ,)
    ELAPSED=$(grep 'seconds time elapsed' perf_stat_summary.txt | awk '{print $1}')
    CPU_UTIL=$(awk -v t="$UTIL" -v e="$ELAPSED" -v n="$THREAD_COUNT" 'BEGIN { printf("%.2f%%", 100*t/(e*1000*n)) }')
    echo "Estimated CPU utilization: $CPU_UTIL" >> perf_stat_summary.txt

    # Run interval-based perf stat (CSV format, every 100 ms)
    perf stat -e task-clock,cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
        -I 100 -x, -o perf_stat_timeline.csv \
        ../../max_char

    echo "Performance summary for $THREAD_COUNT threads saved to perf_stat_summary.txt"

    # Go back to the base directory
    cd ../../
done