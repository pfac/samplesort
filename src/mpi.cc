#include <cerrno>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <mpi.h>

#include "ccut-array.hpp"
using ccut::LArray;

#ifdef	TK_PROFILE
#include "tk/stopwatch.hpp"
#endif


void diveven ( long unsigned x , long unsigned y , long unsigned a[] )
{
	long unsigned d;
	long unsigned i;
	for ( d = 0 ; x > y ; ++d )
		x -= y;
	for ( i = 0 ; i < y ; ++i )
		a[i] = ( x > i ) ? d + 1 : d;
}



int comparel (const void * a, const void * b)
{
  return ( *(long*)a - *(long*)b );
}



int main ( int argc , char *argv[] )
{
	if ( argc < 2 )
	{
		std::cerr
			<<	"Usage: "
			<<	argv[0]
			<<	" <int_array_filename>"
			<<	std::endl;
		throw(EINVAL);
	}


	int mpi_rank;//	process rank
	int mpi_size;//	number of processes
	long * partition;
	long unsigned partsize;
	long unsigned nkeys;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size (MPI_COMM_WORLD, &mpi_size);

#ifdef	TK_PROFILE
	tk::Stopwatch sw;
	if (!mpi_rank)
		sw.start();
#endif

		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	if ( r == mpi_rank )
		// 		cerr << getpid() << "\tProcess " << mpi_rank << '/' << mpi_size << endl;
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// }

	if ( mpi_rank )
	{
		MPI_Status status;
		MPI_Recv (&partsize, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD, &status);
		partition = new long[partsize];
		MPI_Recv (partition, partsize, MPI_LONG, 0, 1, MPI_COMM_WORLD, &status);
		MPI_Recv (&nkeys, 1, MPI_UNSIGNED_LONG, 0, 2, MPI_COMM_WORLD, &status);
	}
	else
	{
		LArray keys( argv[1] );

		// long * keys = lkeys.data;
		nkeys = keys.length;

		long unsigned * psizes = new long unsigned[mpi_size];
		long unsigned * pfirsts = new long unsigned[mpi_size];

		diveven (nkeys, mpi_size, psizes);

		
		pfirsts[0] = 0;
		for ( int r = 1 ; r < mpi_size ; ++r )
			pfirsts[r] = pfirsts[r-1] + psizes[r-1];

			// //debug
			// for ( int r = 0 ; r < mpi_size ; ++r )
			// 	cerr << r << '\t' << psizes[r] << '\t' << pfirsts[r] << endl;

		for ( int r = 1 ; r < mpi_size ; ++r )
		{
			MPI_Send (psizes + r, 1, MPI_UNSIGNED_LONG, r, 0, MPI_COMM_WORLD);
			MPI_Send (keys.data + pfirsts[r], psizes[r], MPI_LONG, r, 1, MPI_COMM_WORLD);
			MPI_Send (&nkeys, 1, MPI_UNSIGNED_LONG, r, 2, MPI_COMM_WORLD);
		}

		// partition = keys.data;
		// partsize = psizes[0];
		partsize = psizes[0];
		partition = new long[partsize];
		memcpy (partition, keys.data, sizeof(long) * partsize);
		
		delete[] psizes;
		delete[] pfirsts;
	}
	// MPI_Bcast (&nkeys, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for ( long unsigned n = 0 ; n < partsize ; ++n )
		// 			cerr << partition[n] << ' ';
		// 		cerr << endl;
		// 	}
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// }


	//	LOCAL SORT
	qsort (partition, partsize, sizeof(long), comparel);
		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for ( long unsigned n = 0 ; n < partsize ; ++n )
		// 			cerr << partition[n] << ' ';
		// 		cerr << endl;
		// 	}
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// }
		// MPI_Barrier (MPI_COMM_WORLD);


	//	SAMPLING
	long unsigned mpi_size_sqr = mpi_size * mpi_size;
	// long unsigned sampregint = nkeys / mpi_size_sqr;
	long unsigned * samp_ints = new long unsigned[mpi_size];
	long * samples;

	if (mpi_rank)
		samples = new long[mpi_size];
	else
		samples = new long[mpi_size_sqr];

	diveven (partsize, mpi_size, samp_ints);
		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for (int n = 0; n < mpi_size; ++n)
		// 			cerr << samp_ints[n] << ' ';
		// 		cerr << endl;
		// 	}
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// }

	{
		long unsigned w = 0;
		for ( int r = 0 ; r < mpi_size ; ++r )
		{
			samples[r] = partition[w];
			w += samp_ints[r];
		}
	}
		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for ( int n = 0 ; n < mpi_size ; ++n )
		// 			cerr << samples[n] << ' ';
		// 		cerr << endl;
		// 	}
		// }

	delete[] samp_ints;


	//	GATHER SAMPLES
	if ( mpi_rank )
		MPI_Send (samples, mpi_size, MPI_UNSIGNED_LONG, 0, 3, MPI_COMM_WORLD);
	else
	{
		MPI_Status status;
		for (int r = 1; r < mpi_size; ++r)
			MPI_Recv (samples + r * mpi_size, mpi_size, MPI_UNSIGNED_LONG, r, 3, MPI_COMM_WORLD, &status);
			// //debug
			// cerr << "Process 0" << endl << '\t';
			// for (long unsigned s = 0; s < mpi_size_sqr; ++s)
			// 	cerr << samples[s] << ' ';
			// cerr << endl;

		//	ORDER SAMPLES
		qsort (samples, mpi_size_sqr, sizeof(long), comparel);
			// //debug
			// cerr << "Process 0" << endl << '\t';
			// for (long unsigned s = 0; s < mpi_size_sqr; ++s)
			// 	cerr << samples[s] << ' ';
			// cerr << endl;
	}


	//	FIND PIVOTS
	int mpi_size_lst = mpi_size - 1;
	long * pivots = new long[mpi_size_lst];

	if (mpi_rank)
	{
		MPI_Status status;
		MPI_Recv (pivots, mpi_size - 1, MPI_LONG, 0, 4, MPI_COMM_WORLD, &status);
	}
	else
	{
		long unsigned * piv_ints = new long unsigned[mpi_size];
		// long unsigned mpi_size_hlf = mpi_size / 2;

		diveven (mpi_size_sqr, mpi_size, piv_ints);
			// //debug
			// cerr << "Pivot intervals" << endl << '\t';
			// for (int p = 0; p < mpi_size_lst; ++p)
			// 	cerr << piv_ints[p] << ' ';
			// cerr << endl;


		long unsigned w = piv_ints[0];
		for (int p = 1; p < mpi_size; ++p)
		{
			pivots[p-1] = samples[w];
			w += piv_ints[p];
		}
			// //debug
			// cerr << "Pivots" << endl << '\t';
			// for (int p = 0; p < mpi_size_lst; ++p)
			// 	cerr << pivots[p] << ' ';
			// cerr << endl;

		for (int r = 1; r < mpi_size; ++r)
			MPI_Send (pivots, mpi_size_lst, MPI_LONG, r, 4, MPI_COMM_WORLD);

		delete[] piv_ints;
	}
	// MPI_Bcast (pivots, mpi_size - 1, MPI_LONG, 0, MPI_COMM_WORLD);
		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for (int n = 0; n < mpi_size - 1; ++n)
		// 			cerr << pivots[n] << ' ';
		// 		cerr << endl;
		// 	}
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// }



	//	SLICES
	long unsigned * sfirsts = new long unsigned[mpi_size];
	long unsigned * ssizes = new long unsigned[mpi_size];


		// cerr << "Process " << mpi_rank << endl << '\t';
		// for ( long unsigned n = 0 ; n < partsize ; ++n )
		// 	cerr << partition[n] << ' ';
		// cerr << endl;
	{
		int i = 0;
		long unsigned j = 0;
		long unsigned n = 0;
		sfirsts[0] = 0;
		while (n < partsize)
		{
			if (i < mpi_size_lst && partition[n] > pivots[i])
			{
				ssizes[i] = n - j;
				j = n;
				i += 1;
				sfirsts[i] = n;
			}
			else
				n += 1;
		}
		ssizes[i] = n - j;
		for (i += 1; i < mpi_size; ++i)
		{
			sfirsts[i] = n;
			ssizes[i] = 0;
		}
	}
		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for (int n = 0; n < mpi_size; ++n)
		// 			cerr << sfirsts[n] << '/' << ssizes[n] << ' ';
		// 		cerr << endl;
		// 	}
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// }

		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for (int s = 0; s < mpi_size; ++s)
		// 		{
		// 			cerr << "Slice " << s << endl << "\t\t";
		// 			for (long unsigned n = 0; n < ssizes[s]; ++n)
		// 				cerr << partition[sfirsts[s] + n] << ' ';
		// 			cerr << endl;
		// 		}
		// 	}
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// }


	//	TRADE SLICES
	long unsigned * inc_sizes = new long unsigned[mpi_size];
	partsize = inc_sizes[mpi_rank] = ssizes[mpi_rank];

	for (int r = 0; r < mpi_size; ++r)
		if (r == mpi_rank)
		{
			for (int p = 0; p < mpi_size; ++p)
				if (p != r)
					MPI_Send (ssizes+p, 1, MPI_UNSIGNED_LONG, p, 500 + r, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Status status;
			long unsigned tmp_size;
			MPI_Recv (&tmp_size, 1, MPI_UNSIGNED_LONG, r, 500 + r, MPI_COMM_WORLD, &status);
			inc_sizes[r] = tmp_size;
			partsize += tmp_size;
		}

		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t' << partsize << endl;
		// 	}
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// }

		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for (int n = 0 ; n < mpi_size ; ++n )
		// 			cerr << inc_sizes[n] << ' ';
		// 		cerr << endl;
		// 	}
		// }

	long * partfinal = new long[partsize];
	long unsigned w = 0;

	for (int r = 0; r < mpi_size; ++r)
	{
		if (r == mpi_rank)
		{
			for (int p = 0; p < mpi_size; ++p)
				if (p != r)
					MPI_Send (partition + sfirsts[p], ssizes[p], MPI_LONG, p, 600 + r, MPI_COMM_WORLD);
			for (unsigned long n = 0; n < ssizes[mpi_rank]; ++n)
				partfinal[w+n] = partition[sfirsts[mpi_rank]+n];
		}
		else
		{
			MPI_Status status;
			MPI_Recv (partfinal + w, inc_sizes[r], MPI_LONG, r, 600 + r, MPI_COMM_WORLD, &status);
		}
		w += inc_sizes[r];
	}

		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for (long unsigned n = 0 ; n < partsize ; ++n )
		// 			cerr << partfinal[n] << ' ';
		// 		cerr << endl;
		// 	}
		// }

	//	SORT FINAL PARTITION
	qsort (partfinal, partsize, sizeof(long), comparel);
		// //debug
		// for ( int r = 0 ; r < mpi_size ; ++r )
		// {
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// 	if ( r == mpi_rank )
		// 	{
		// 		cerr << "Process " << mpi_rank << endl << '\t';
		// 		for (long unsigned n = 0 ; n < partsize ; ++n )
		// 			cerr << partfinal[n] << ' ';
		// 		cerr << endl;
		// 	}
		// 	MPI_Barrier (MPI_COMM_WORLD);
		// }


	//	GATHER
	if (mpi_rank)
	{
		MPI_Send (&partsize, 1, MPI_UNSIGNED_LONG, 0, 7, MPI_COMM_WORLD);
		MPI_Send (partfinal, partsize, MPI_LONG, 0, 8, MPI_COMM_WORLD);
	}
	else
	{
		long * keys = new long[nkeys];
		long unsigned loaded;

		for (long unsigned n = 0; n < partsize; ++n)
			keys[n] = partfinal[n];

		loaded = partsize;
		for (int r = 1; r < mpi_size; ++r)
		{
			MPI_Status status;
			MPI_Recv (&partsize, 1, MPI_UNSIGNED_LONG, r, 7, MPI_COMM_WORLD, &status);
			MPI_Recv (keys + loaded, partsize, MPI_LONG, r, 8, MPI_COMM_WORLD, &status);
			loaded += partsize;
		}

		// cerr << "Final answer" << endl << '\t';
		// for (unsigned long n = 0; n < nkeys; ++n)
		// 	cerr << keys[n] << ' ';
		// cerr << endl;

#ifdef	TK_PROFILE
		sw.stop();
		cout << sw.last().microseconds() << endl;
#else
		for (long unsigned n = 0; n < nkeys; ++n)
			cout << keys[n] << endl;
#endif


		delete[] keys;
	}


	delete[] partfinal;
	delete[] inc_sizes;
	delete[] sfirsts;
	delete[] ssizes;
	delete[] pivots;
	delete[] samples;
	delete[] partition;


	MPI_Finalize();
	
	return 0;
}
