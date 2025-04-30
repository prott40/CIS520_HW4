#!/bin/bash
#SBATCH --mem-per-cpu=512MB
#SBATCH --cpus-per-task=8
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --constraint=moles

# Define the thread configurations
THREADS=("1" "2" "4" "8" "16")

# Clean previous outputs
rm -rf results
mkdir results

# Loop over each thread count
for THREAD_COUNT in "${THREADS[@]}"; do
    # Create a directory for each thread count
    mkdir results/threads_$THREAD_COUNT

    # Set OpenMP environment
    export OMP_NUM_THREADS=$THREAD_COUNT
    export GOMP_CPU_AFFINITY="0-15"
    export OMP_STACKSIZE=64M

    # Compile OpenMP code
    gcc -fopenmp -O2 -g -march=native -o max_char openmpmech.c

    # Navigate to the results directory
    cd results/threads_$THREAD_COUNT

    # Run basic perf stat (one-line summary)
    perf stat -e cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
        -o perf_stat_summary.txt \
        ../../max_char

    # Run interval-based perf stat (CSV format, every 100 ms)
    perf stat -e cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
        -I 100 -x, -o perf_stat_timeline.csv \
        ../../max_char

    # Generate a summarized CSV from raw perf output
    awk -F, '
    BEGIN {
        print "Time(ms),Event,Count"
    }
    $1 ~ /^[0-9]+$/ {
        print $1 "," $3 "," $2
    }
    ' perf_stat_timeline.csv > perf_data_summary.csv

    # Optional: Print where results were saved
    echo "Performance summary for $THREAD_COUNT threads saved to perf_stat_summary.txt"
    echo "Detailed timeline data for $THREAD_COUNT threads saved to perf_data_summary.csv"

    # Go back to the base directory
    cd ../../
done