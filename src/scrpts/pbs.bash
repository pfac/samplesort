#!/bin/bash
#		pbs script for single node jobs
#
#PBS -l walltime=1:00:00
#PBS -M dev+pbs@pfac.info
#PBS -m bea
#

#	EXEC		executable to run
#	INPUT		input file, ascii matrix format (see int.txt)
#	PARTITIONS	number of parallel partitions (defaults to the number of processors available)
#	RUNS		number of runs (defaults to 1)

if [ ! "$PARTITIONS" ]; then PARTITIONS=`cat "$PBS_NODEFILE" | wc -l`; fi;
if [ ! "$RUNS" ]; then RUNS="1"; fi;

cd "$PBS_O_WORKDIR";

R="1";
while [ "$R" -le "$RUNS" ];
do
	$EXEC "$INPUT" "$PARTITIONS"
	R=$(( $R + 1 ));
done;
