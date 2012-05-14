#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Sequential Samplesort
#
#PBS -N samplesort-omp06
#
#PBS -l nodes=1:hex:ppn=24
#PBS -l walltime=5:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/omp06.hex.err
#PBS -o ../../data/pbs.out/omp06.hex.out
#
runs=10;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	./omp ../../data/input/64MB.txt 6;
done;

