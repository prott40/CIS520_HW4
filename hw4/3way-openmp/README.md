FOR A RUN OF JUST VERSION 1 OVER MULTIPLE THREADS, USE "sbatch quick_run.sh".
THIS WILL CREATE A DIRECTORY CALLED "quick_results" WITH SUBDIRECTORIES CONTAINING THE RESULTS

openmpmechV0: V1 but doesn't use memory mapped I/O, just loads that many lines to memory
openmpmechV1: Parallelizes the max_char loop over a memory mapped txt file.
openmcmechV2: V1 AND parallelizes the memory mapping section that finds line lengths and pointers. (V2 FAILED DUE TO UNRESOLVED SEG FAULT)
openmcmechV3: V1 AND parallelizes the output of the max_chars

commands
make: This is release mode uses -O2
make DEBUG: This uses -O0 and -g

sbatch run_openmp.sh: This creates results folder and the previous results are saved as previous_results.