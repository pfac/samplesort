#!/bin/bash
#		pbs script for single node jobs
#
#PBS -l walltime=2:00:00
#PBS -M dev+pbs@pfac.info
#PBS -m bea
#

#	EXEC		executable to run
#	INPUT		input file, ascii matrix format (see int.txt)
#	PARTMIN		minimum number of parallel partitions (defaults 1)
#	PARTMAX		maximum number of parallel partitions (defaults to 8192)
#	RUNS		number of runs (defaults to 1)

if [ ! "$PARTMIN" ]; then PARTMIN="1"; fi;
if [ ! "$PARTMAX" ]; then PARTMAX="8192"; fi;
if [ ! "$RUNS" ]; then RUNS="10"; fi;

cd "$PBS_O_WORKDIR";

PARTITIONS="$PARTMIN";
while [ "$PARTITIONS" -le "$PARTMAX" ];
do
	R="1";
	echo "$PARTITIONS partition(s)";
	while [ "$R" -le "$RUNS" ];
	do
		if [ "$THREADS" ];
		then
			$EXEC "$INPUT" "$PARTITIONS" "$THREADS";
		else
			$EXEC "$INPUT" "$PARTITIONS";
		fi;
		R=$(( $R + 1 ));
	done;
	echo;
	PARTITIONS=$(( $PARTITIONS * 2 ));
done;
