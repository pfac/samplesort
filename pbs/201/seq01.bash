#!/bin/bash
# PBS Script for the SeARCH Group hex
# 	Sequential Samplesort
#
#PBS -N samplesort-seq01
#
#PBS -l nodes=1:r201:ppn=4
#PBS -l walltime=5:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e ../../data/pbs.out/seq01.201.err
#PBS -o ../../data/pbs.out/seq01.201.out
#
runs=10;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	./seq ../../data/input/64MB.txt 1;
done;

