#!/bin/bash
# PBS Script for the SeARCH Group 601
# 	Sequential Samplesort
#
#PBS -N samplesort-omp72
#
#PBS -l nodes=1:hex:ppn=24
#PBS -l walltime=30:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/omp72.601.err
#PBS -o ../../data/pbs.out/omp72.601.out
#
runs=10;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	./omp ../../data/input/512MB.txt 72;
done;

