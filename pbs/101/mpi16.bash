#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Sequential Samplesort
#
#PBS -N samplesort-mpi16
#
#PBS -l nodes=8:r101:ppn=2
#PBS -l walltime=30:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/mpi16.101.err
#PBS -o ../../data/pbs.out/mpi16.101.out
#
#PBS -V
#
runs=10;
cd "$PBS_O_WORKDIR";
module load gnu/openmpi;
for (( i = 0 ; i < $runs ; ++i ));
do
	mpirun -np 16 -machinefile $PBS_NODEFILE -loadbalance mpi ../../data/input/512MB.txt;
done;

