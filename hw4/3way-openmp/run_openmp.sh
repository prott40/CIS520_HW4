#!/bin/bash
#SBATCH --mem-per-cpu=512MB           # Memory per core
#SBATCH --cpus-per-task=1       # Number of CPU cores
#SBATCH --nodes=1                  # Run on 1 node
#SBATCH --ntasks=1               # 1 task total (read the wikidump)
#SBATCH --constraint=moles       # Use moles 
# Compile with flags
gcc -fopenmp -02 -g -o max_char openmpmech.c
# Run original perf stat for basic metrics
perf stat -e cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
    -o perf_stat_output.txt \
    ./max_char
# Run perf stat with periodic output to create data for gnuplot
perf stat -e cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
    -I 100 -x, -o perf_data.csv \
    ./max_char
# Record data for flamegraph
perf record -g -o perf_record.data ./max_char
# Generate a readable report
perf report -i perf_record.data > perf_report.txt