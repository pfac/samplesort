#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Sequential Samplesort
#
#PBS -N samplesort-qsort
#
#PBS -l nodes=1:hex:ppn=24
#PBS -l walltime=30:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/qsort.hex.err
#PBS -o ../../data/pbs.out/qsort.hex.out
#
runs=10;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	./qsort ../../data/input/512MB.txt 1;
done;

