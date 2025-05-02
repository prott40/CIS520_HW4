#!/bin/bash -l
#SBATCH --mem-per-cpu=512MB           ## Memory per core
#SBATCH --cpus-per-task=1       ## Number of CPU cores
#SBATCH --nodes=1                  ## Run on 1 node
#SBATCH --ntasks=1               ## 1 task total (read the wikidump)
#SBATCH --constraint=moles       ## Use moles 
##OUTPUT="/homes/acerna/CIS520/hw4/3way-mpi/output/time.csv"
##PRO="./max_char"
##run all 10 tests
sbatch --mem-per-cpu=512MB --cpus-per-task=1 --nodes=1 --ntasks=1 --constraint=moles ./testbatch.sh

