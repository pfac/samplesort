#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Sequential Samplesort
#
#PBS -N samplesort-mpi64
#
#PBS -l nodes=64:ppn=1
#PBS -l walltime=30:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/mpi64.all.err
#PBS -o ../../data/pbs.out/mpi64.all.out
#
#PBS -V
#
runs=10;
cd "$PBS_O_WORKDIR";
module load gnu/openmpi;
for (( i = 0 ; i < $runs ; ++i ));
do
	mpirun -np 64 -machinefile $PBS_NODEFILE -loadbalance mpi ../../data/input/512MB.txt;
done;

