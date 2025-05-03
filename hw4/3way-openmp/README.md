openmpmechV0: V1 but doesn't use memory mapped I/O, just loads that many lines to memory
openmpmechV1: Parallelizes the max_char loop over a memory mapped txt file.
openmcmechV2: V1 AND parallelizes the memory mapping section that finds line lengths and pointers.
openmcmechV3: V1 AND parallelizes the output of the max_chars

make: release mode uses -O2
make DEBUG: uses -O0 and -g

sbatch run_openmp.sh to create results folder and the previous results are saved as previous_results.

srun act strangely