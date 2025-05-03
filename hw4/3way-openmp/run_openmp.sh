#!/bin/bash
#SBATCH --mem-per-cpu=512MB
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --constraint=moles

THREADS=("1" "2" "4" "8" "16" "20")

VERSIONS=("V0" "V1" "V2" "V3")

SPECIAL_LINE_COUNTS=("1000" "10000" "100000")

rm -rf previous_results
if [ -d results ]; then
    mv results previous_results
fi
mkdir results

for VERSION in "${VERSIONS[@]}"; do
    gcc -fopenmp -O2 -g -march=native -o max_char$VERSION openmpmech$VERSION.c
done

for VERSION in "${VERSIONS[@]}"; do

    # variable Line amounts
    for LINE_COUNT in "${SPECIAL_LINE_COUNTS[@]}"; do
        for RUN in {1..10}; do
            DIR="results/$VERSION/lines_${LINE_COUNT}_threads_16/run_$RUN"
            mkdir -p "$DIR"

            export OMP_NUM_THREADS=16
            export OMP_STACKSIZE=64M

            cd "$DIR"
            touch perf_stat_summary.txt

            #run to append results from the block time
            #../../../../max_char$VERSION $LINE_COUNT

            /usr/bin/time -v perf stat -e task-clock,cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
            -o perf_stat_tmp.txt \
            ../../../../max_char$VERSION $LINE_COUNT 2> time_output.txt

            # append perf and time outputs
            cat perf_stat_tmp.txt >> perf_stat_summary.txt
            echo "" >> perf_stat_summary.txt
            grep "Maximum resident set size" time_output.txt >> perf_stat_summary.txt

            rm perf_stat_tmp.txt time_output.txt

            UTIL=$(grep 'task-clock' perf_stat_summary.txt | awk '{print $1}' | tr -d ,)
            ELAPSED=$(grep 'seconds time elapsed' perf_stat_summary.txt | awk '{print $1}')
            CPU_UTIL=$(awk -v t="$UTIL" -v e="$ELAPSED" -v n="16" 'BEGIN { printf("%.2f%%", 100*t/(e*1000*n)) }')
            echo "Estimated CPU utilization: $CPU_UTIL" >> perf_stat_summary.txt

            echo "Special test: $VERSION, $LINE_COUNT lines, 16 threads, run $RUN saved to $DIR"
            cd ../../../../
        done
    done

    #variable thread amounts (1000000 input)
    for THREAD_COUNT in "${THREADS[@]}"; do
        for RUN in {1..10}; do
            DIR="results/$VERSION/threads_${THREAD_COUNT}/run_$RUN"
            mkdir -p "$DIR"

            export OMP_NUM_THREADS=$THREAD_COUNT
            export OMP_STACKSIZE=64M

            cd "$DIR"
            touch perf_stat_summary.txt

            #run to append results from the block time
            #../../../../max_char$VERSION

            /usr/bin/time -v perf stat -e task-clock,cycles,instructions,cache-misses,context-switches,minor-faults,major-faults \
            -o perf_stat_tmp.txt \
            ../../../../max_char$VERSION 2> time_output.txt

            # append perf and time outputs
            cat perf_stat_tmp.txt >> perf_stat_summary.txt
            echo "" >> perf_stat_summary.txt
            grep "Maximum resident set size" time_output.txt >> perf_stat_summary.txt

            rm perf_stat_tmp.txt time_output.txt

            UTIL=$(grep 'task-clock' perf_stat_summary.txt | awk '{print $1}' | tr -d ,)
            ELAPSED=$(grep 'seconds time elapsed' perf_stat_summary.txt | awk '{print $1}')
            CPU_UTIL=$(awk -v t="$UTIL" -v e="$ELAPSED" -v n="$THREAD_COUNT" 'BEGIN { printf("%.2f%%", 100*t/(e*1000*n)) }')
            echo "Estimated CPU utilization: $CPU_UTIL" >> perf_stat_summary.txt

            echo "Thread test: $VERSION, $THREAD_COUNT threads, run $RUN saved to $DIR"
            cd ../../../../
        done
    done

done
