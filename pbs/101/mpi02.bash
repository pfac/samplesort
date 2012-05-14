#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Sequential Samplesort
#
#PBS -N samplesort-mpi02
#
#PBS -l nodes=1:r101:ppn=2
#PBS -l walltime=5:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/mpi02.101.err
#PBS -o ../../data/pbs.out/mpi02.101.out
#
#PBS -V
#
runs=10;
cd "$PBS_O_WORKDIR";
module load gnu/openmpi;
for (( i = 0 ; i < $runs ; ++i ));
do
	mpirun -np 2 -machinefile $PBS_NODEFILE -loadbalance mpi ../../data/input/64MB.txt;
done;

