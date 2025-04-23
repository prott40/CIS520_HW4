#!/bin/bash -l
#SBATCH --mem-per-cpu=512MB           ## Memory per core
#SBATCH --cpus-per-task=4       ## Number of CPU cores
#SBATCH --nodes=1                  ## Run on 1 node
#SBATCH --ntasks=1               ## 1 task total (read the wikidump)
#SBATCH --constraint=moles       ## Use moles --field-separator=','
#SBATCH --output=slurm-%j.out ## keep outfile in file
# Run perf stat (collect time elapsed & CPU) and output to perf.txt
perf stat --repeat=10 -o perf.txt ./max_char > /dev/null 2>&1

# Run /usr/bin/time to get max RSS
/usr/bin/time -f "Elapsed:%e\nMaxRSS:%M\nCPU:%P" -o time.txt ./max_char > /dev/null 2>&1

# Extract average time elapsed & its stddev
total_time=$(grep "seconds time elapsed" perf.txt | awk '{print $1}')
time_stddev=$(grep "seconds time elapsed" perf.txt | grep -oP '\(\s*\+-\s*\K[0-9.]+%' | head -1)

# Extract average CPU utilized & its stddev
cpu_util=$(grep "task-clock" perf.txt | awk -F'#' '{print $2}' | awk '{print $1}')
cpu_stddev=$(grep "task-clock" perf.txt | grep -oP '\(\s*\+-\s*\K[0-9.]+%' | head -1)

# Extract Max RSS from /usr/bin/time
max_rss=$(grep "MaxRSS" time.txt | awk -F ':' '{print $2}' | tr -d ' ')

# Output CSV with header if file doesn't exist
output_file="/homes/pmrottin/hw4/3way-pthread/lines_1M/4core/thread16/time.csv"
if [ ! -s "$output_file" ]; then
    echo "TotalTime(s),TimeStdDev(%),CPUUtil(%),CPUStdDev(%),MaxRSS(KB)" > "$output_file"
fi

# Append the extracted values to the CSV file
echo "$total_time,$time_stddev,$cpu_util,$cpu_stddev,$max_rss" >> "$output_file"