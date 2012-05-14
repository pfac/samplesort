#!/bin/bash
# PBS Script for the SeARCH Group 511
# 	Sequential Samplesort
#
#PBS -N samplesort-mpi48
#
#PBS -l nodes=2:r511:ppn=24
#PBS -l walltime=30:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/mpi48.511.err
#PBS -o ../../data/pbs.out/mpi48.511.out
#
#PBS -V
#
runs=10;
cd "$PBS_O_WORKDIR";
module load gnu/openmpi;
for (( i = 0 ; i < $runs ; ++i ));
do
	mpirun -np 48 -machinefile $PBS_NODEFILE -loadbalance mpi ../../data/input/64MB.txt;
done;

