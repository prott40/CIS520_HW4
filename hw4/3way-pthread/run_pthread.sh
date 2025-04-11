#!/bin/bash -l
#SBATCH --job-name=test_pthread
#SBATCH --time=00:30:00
#SBATCH --mem=2G
#SBATCH --cpus-per-task=4 
#SBATCH --ntasks=1
#SBATCH --output=res_pthread.out
module load foss        
gcc -pthread -O2 -o max_char pthreadmech.c
./max_char