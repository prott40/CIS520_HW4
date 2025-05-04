#!/bin/bash

OUTPUT_FILE="results_summary.csv"
echo "Version,LineCount,Threads,Run,TaskClock,Cycles,Instructions,CacheMisses,ContextSwitches,MinorFaults,MajorFaults,ElapsedTime,CPUUtilization,MaxRSS,MaxCharTime" > "$OUTPUT_FILE"

# results loop
for VERSION_DIR in results/*; do
    VERSION=$(basename "$VERSION_DIR")

    for CONFIG_DIR in "$VERSION_DIR"/*; do
        CONFIG=$(basename "$CONFIG_DIR")

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
            PERF_FILE="$RUN_DIR/perf_stat_summary.txt"
            LOG_FILE="$RUN_DIR/custom_log.txt"

            if [ -f "$PERF_FILE" ]; then
                MAX_CHAR_TIME=""
                if [ -f "$LOG_FILE" ]; then
                    MAX_CHAR_TIME=$(grep -i 'Parallel max-char computation time' "$LOG_FILE" | awk '{print $(NF-1)}')
                fi

                TASK_CLOCK=$(grep 'task-clock' "$PERF_FILE" | awk '{print $1}' | tr -d ,)
                CYCLES=$(grep 'cycles' "$PERF_FILE" | awk '{print $1}' | tr -d ,)
                INSTR=$(grep 'instructions' "$PERF_FILE" | awk '{print $1}' | tr -d ,)
                CACHE_MISS=$(grep 'cache-misses' "$PERF_FILE" | awk '{print $1}' | tr -d ,)
                CTX_SWITCH=$(grep 'context-switches' "$PERF_FILE" | awk '{print $1}' | tr -d ,)
                MIN_FAULTS=$(grep 'minor-faults' "$PERF_FILE" | awk '{print $1}' | tr -d ,)
                MAJ_FAULTS=$(grep 'major-faults' "$PERF_FILE" | awk '{print $1}' | tr -d ,)
                ELAPSED=$(grep 'seconds time elapsed' "$PERF_FILE" | awk '{print $1}')
                UTIL=$(grep 'Estimated CPU utilization' "$PERF_FILE" | awk -F': ' '{print $2}' | tr -d '%')
                MAX_RSS=$(grep 'Maximum resident set size' "$PERF_FILE" | awk -F': ' '{print $2}')

                echo "$VERSION,$LINE_COUNT,$THREADS,$RUN,$TASK_CLOCK,$CYCLES,$INSTR,$CACHE_MISS,$CTX_SWITCH,$MIN_FAULTS,$MAJ_FAULTS,$ELAPSED,$UTIL,$MAX_RSS,$MAX_CHAR_TIME" >> "$OUTPUT_FILE"
            fi
        done
    done
done

echo "CSV written to $OUTPUT_FILE"
