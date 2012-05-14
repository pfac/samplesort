#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Sequential Samplesort
#
#PBS -N samplesort-omp01
#
#PBS -l nodes=1:r101:ppn=2
#PBS -l walltime=5:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/omp01.101.err
#PBS -o ../../data/pbs.out/omp01.101.out
#
runs=10;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	./omp ../../data/input/64MB.txt 1;
done;

