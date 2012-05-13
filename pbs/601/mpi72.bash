#!/bin/bash
# PBS Script for the SeARCH Group 601
# 	Sequential Samplesort
#
#PBS -N samplesort-mpi72
#
#PBS -l nodes=3:r601:ppn=24
#PBS -l walltime=1:00:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/mpi72.601.err
#PBS -o ../../data/pbs.out/mpi72.601.out
#
runs=10;
cd "$PBS_O_WORKDIR";
module load gnu/openmpi;
for (( i = 0 ; i < $runs ; ++i ));
do
	mpirun -np 72 -machinefile $PBS_NODEFILE -loadbalance mpi input/10M.txt;
done;

