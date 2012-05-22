#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Distributed Samplesort
#
#PBS -N samplesort-mpi002
#
#PBS -l nodes=8:r101:ppn=4
#PBS -l walltime=30:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/mpi002.101.err
#PBS -o ../../data/pbs.out/mpi002.101.out
#
#PBS -V
#
runs=20;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	mpirun -np 2 -machinefile $PBS_NODEFILE -loadbalance mpi ../../data/input/512MB.txt;
done;

