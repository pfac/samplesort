#PBS -N test_mpi_mx
#PBS -l walltime=0:30:00
#PBS -l nodes=compute-101-25+compute-101-26
#PBS -j oe
#PBS -m abe -M pdrcosta90@gmail.com

cd $PBS_O_WORKDIR

/opt/openmpi-myrinet_mx/bin/mpirun -loadbalance -H compute-101-25,compute-101-26 -np 2 mpi ../../data/512MB.txt
