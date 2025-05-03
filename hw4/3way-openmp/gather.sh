#!/bin/bash

OUTPUT_FILE="results_summary.csv"
echo "Version,LineCount,Threads,Run,TaskClock,Cycles,Instructions,CacheMisses,ContextSwitches,MinorFaults,MajorFaults,ElapsedTime,CPUUtilization,MaxRSS,MaxCharTime,AppTime" > $OUTPUT_FILE

# results loop
for VERSION_DIR in results/*; do
    VERSION=$(basename "$VERSION_DIR")

    for CONFIG_DIR in "$VERSION_DIR"/*; do
        CONFIG=$(basename "$CONFIG_DIR")

        # extract LineCount and Thread count from directory name
        if [[ "$CONFIG" == lines_* ]]; then
            LINE_COUNT=$(echo "$CONFIG" | sed -E 's/lines_([0-9]+)_threads_([0-9]+)/\1/')
            THREADS=$(echo "$CONFIG" | sed -E 's/lines_([0-9]+)_threads_([0-9]+)/\2/')
        elif [[ "$CONFIG" == threads_* ]]; then
            LINE_COUNT="1000000"
            THREADS=$(echo "$CONFIG" | sed -E 's/threads_([0-9]+)/\1/')
        else
            continue
        fi

        for RUN_DIR in "$CONFIG_DIR"/run_*; do
            RUN=$(basename "$RUN_DIR" | cut -d'_' -f2)
            FILE="$RUN_DIR/perf_stat_summary.txt"

            if [ -f "$FILE" ]; then
                TASK_CLOCK=$(grep 'task-clock' "$FILE" | awk '{print $1}' | tr -d ,)
                CYCLES=$(grep 'cycles' "$FILE" | awk '{print $1}' | tr -d ,)
                INSTR=$(grep 'instructions' "$FILE" | awk '{print $1}' | tr -d ,)
                CACHE_MISS=$(grep 'cache-misses' "$FILE" | awk '{print $1}' | tr -d ,)
                CTX_SWITCH=$(grep 'context-switches' "$FILE" | awk '{print $1}' | tr -d ,)
                MIN_FAULTS=$(grep 'minor-faults' "$FILE" | awk '{print $1}' | tr -d ,)
                MAJ_FAULTS=$(grep 'major-faults' "$FILE" | awk '{print $1}' | tr -d ,)
                ELAPSED=$(grep 'seconds time elapsed' "$FILE" | awk '{print $1}')
                UTIL=$(grep 'Estimated CPU utilization' "$FILE" | awk '{print $(NF)}')
                MAX_RSS=$(grep -i 'max resident set size' "$FILE" | awk '{print $NF}')
                MAX_CHAR_TIME=$(grep -i 'Parallel max-char computation time' "$FILE" | awk '{print $(NF-1)}')
                APP_TIME=$(grep -i 'Application time' "$FILE" | awk '{print $(NF-1)}')

                echo "$VERSION,$LINE_COUNT,$THREADS,$RUN,$TASK_CLOCK,$CYCLES,$INSTR,$CACHE_MISS,$CTX_SWITCH,$MIN_FAULTS,$MAJ_FAULTS,$ELAPSED,$UTIL,$MAX_RSS,$MAX_CHAR_TIME,$APP_TIME" >> $OUTPUT_FILE
            fi
        done
    done
done

echo "CSV written to $OUTPUT_FILE"
