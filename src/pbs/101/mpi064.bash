#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Distributed Samplesort
#
#PBS -N samplesort-mpi064
#
#PBS -l nodes=16:r101:myri:ppn=4
#PBS -l walltime=2:00:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/mpi064.101.err
#PBS -o ../../data/pbs.out/mpi064.101.out
#
#PBS -V
#
runs=100;
cd "$PBS_O_WORKDIR";
N=`cat mpi01.bash | wc -l | grep -o "[[:digit:]]\+"`;
for (( i = 0 ; i < $runs ; ++i ));
do
	mpirun -np $N -machinefile $PBS_NODEFILE -loadbalance mpi ../../data/input/512MB.txt;
done;

