
void psrs_mpi ( long keys[] , long unsigned nkeys )
{
	int mpi_rank;//	process rank
	int mpi_size;//	number of processes
	long * partition;
	long unsigned partsize;

	MPI_Comm_rank (MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size (MPI_COMM_WORLD, &mpi_size);

	if ( mpi_rank )
	{
		MPI_Status status;
		MPI_Recv (&partsize, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD, &status);
		partition = new long[partsize];
		MPI_Recv (partition, partsize, MPI_LONG, 0, 1, MPI_COMM_WORLD, &status);
	}
	else
	{
		psizes = new long unsigned[mpi_size];
		pfirsts = new long unsigned[mpi_size];

		diveven (nkeys, nprocessors, psizes);

		for ( long unsigned r = 1 ; r < mpi_size ; ++r )
		{
			long * array = keys + pfirsts[r];
			MPI_Send (psizes[r], 1, MPI_UNSIGNED_LONG, r, 0, MPI_COMM_WORLD);
			MPI_Send (array, psizes[r], MPI_LONG, r, 1, MPI_COMM_WORLD);
		}

		partition = keys;
		partsize = psizes[0];
		
		delete[] psizes;
		delete[] pfirsts;
	}

	for ( long unsigned r = 0 ; r < mpi_size ; ++r )
	{
		MPI_Barrier (MPI_COMM_WORLD);
		if ( r == mpi_rank )
		{
			cerr << "Process " << mpi_rank << endl << '\t';
			for ( long unsigned n = 0 ; n < partsize ; ++n )
				cerr << partition[n] << ' ';
			cerr << endl;
		}
	}

	if ( mpi_rank )
		delete[] partition;
}

