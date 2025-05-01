openmpmechV1: Parallelizes the max_char loop over a memory mapped txt file.
openmcmechV2: Also parallelizes the memory mapping section that finds line lengths and pointers.
openmcmechV3: Attempts to parallelize the output of the max_chars

make: release mode uses -O2
make DEBUG: uses -O0 and -g

slurm run_openmp.sh to create results folder and the previous results are saved as previous_results.