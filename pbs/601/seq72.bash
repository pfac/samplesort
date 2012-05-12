#!/bin/bash
# PBS Script for the SeARCH Group 601
# 	Sequential Samplesort
#
#PBS -N samplesort-seq72
#
#PBS -l nodes=1:r601:ppn=24
#PBS -l walltime=1:00:00
#
#PBS -M pdrcosta90@gmail.com
#PBS -m abe
#
#PBS -e data/pbs.out/seq72.601.err
#PBS -o data/pbs.out/seq72.601.out
#
runs=10;
cd "$PBS_O_WORKDIR";
for (( i = 0 ; i < $runs ; ++i ));
do
	./seq input/10M.txt 72;
done;

