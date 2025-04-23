#!/bin/bash -l
#SBATCH --mem-per-cpu=512MB           ## Memory per core
#SBATCH --cpus-per-task=2       ## Number of CPU cores
#SBATCH --nodes=1                  ## Run on 1 node
#SBATCH --ntasks=2               ## 1 task total (read the wikidump)
#SBATCH --constraint=moles       ## Use moles 
gcc -pthread -O2 -g -o max_char pthreadmech.c
##run all 10 tests
sbatch --mem-per-cpu=512MB --cpus-per-task=2 --nodes=1 --ntasks=1 --constraint=moles ./1c1tperf.sh


