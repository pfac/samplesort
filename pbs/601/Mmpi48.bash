#!/bin/bash
# PBS Script for the SeARCH Group 601
# 	Sequential Samplesort
#
#PBS -N samplesort-mpi48
#
#PBS -l nodes=2:r601:ppn=24
#PBS -l walltime=1:00:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/mpi48.601.err
#PBS -o ../../data/pbs.out/mpi48.601.out
#
runs=10;
cd "$PBS_O_WORKDIR";
module load gnu/openmpi;
for (( i = 0 ; i < $runs ; ++i ));
do
	mpirun -np 48 -machinefile $PBS_NODEFILE -loadbalance mpi ../../data/input/100M.txt;
done;

