#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Distributed Samplesort
#
#PBS -N samplesort-mpi004
#
#PBS -l nodes=4:r101:myri:ppn=4
#PBS -l walltime=2:00:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/mpi004.101.err
#PBS -o ../../data/pbs.out/mpi004.101.out
#
#PBS -V
#
runs=100;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	/opt/openmpi-myrinet_mx/bin/mpirun -np 4 -machinefile $PBS_NODEFILE -loadbalance mpi ../../data/input/512MB.txt;
done;

