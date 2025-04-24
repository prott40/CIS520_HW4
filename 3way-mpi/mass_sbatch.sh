#!/bin/bash

for i in 1 2 4 8 16
do 
  sbatch --constraint=moles --time=04:00:00 --ntasks-per-node=$i --mem-per-cpu=$((16/$i))G --partition=killable.q --nodes=1 mpi_sbatch.sh
done
