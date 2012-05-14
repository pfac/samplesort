#!/bin/bash
# PBS Script for the SeARCH Group 511
# 	Sequential Samplesort
#
#PBS -N samplesort-seq48
#
#PBS -l nodes=1:r511:ppn=24
#PBS -l walltime=5:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/seq48.511.err
#PBS -o ../../data/pbs.out/seq48.511.out
#
runs=10;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	./seq ../../data/input/64MB.txt 48;
done;

