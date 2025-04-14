Repository for CIS520 HW4

Helpful Links:

MPI Definitions: https://www.mpich.org/static/docs/v3.3/www3/

First download the file and load into BEOCAT

For running the pthreads you must cd into the 3way-pthread directory
cd  hw4/3way-pthread

then run the following command
sbatch --time=1 --mem-per-cpu=512MB --cpus-per-task=4 --ntasks=1 --nodes=1 --constraint=moles ./run_pthread.sh

to see the output use the following command
cat slurm-(the numbers on the job).out

