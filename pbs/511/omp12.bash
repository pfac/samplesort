#!/bin/bash
# PBS Script for the SeARCH Group 511
# 	Sequential Samplesort
#
#PBS -N samplesort-omp12
#
#PBS -l nodes=1:r511:ppn=24
#PBS -l walltime=5:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/omp12.511.err
#PBS -o ../../data/pbs.out/omp12.511.out
#
runs=10;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	./omp ../../data/input/64MB.txt 12;
done;

