#!/bin/bash -l
#SBATCH --mem-per-cpu=512MB           ## Memory per core
#SBATCH --cpus-per-task=1       ## Number of CPU cores
#SBATCH --nodes=1                  ## Run on 1 node
#SBATCH --ntasks=1               ## 1 task total (read the wikidump)
#SBATCH --constraint=moles       ## Use moles 
##OUTPUT="/homes/nemerson74/hw4/CIS520_HW4/hw4/3way-openmp/lines_1M/1core/thread1/1c1tstat.csv"
##PRO="./max_char"
gcc -fopenmp -O2 -g -o max_char openmpmech.c
##run all 10 tests
sbatch --mem-per-cpu=512MB --cpus-per-task=1 --nodes=1 --ntasks=1 --constraint=moles ./1c1tperf.sh