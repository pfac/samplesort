#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <cerrno>
#include <cstdlib>

#include <omp.h>
#include <unistd.h>

#include "ccut-array.hpp"
using ccut::LArray;

#ifdef	TK_PROFILE
#include "tk/stopwatch.hpp"
#endif

#define min(x,y) ((x) < (y) ? (x) : (y))

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
			<<	" <int_array_filename> [<partitions>]"
			<<	std::endl;
		throw(EINVAL);
	}

#ifdef	TK_PROFILE
	tk::Stopwatch sw;
	sw.start();
#endif


	// int processors;
	// if (argc > 2)
	// 	processors = strtol (argv[2], NULL, 0);
	// else
	// 	processors = omp_get_num_procs ();



	// #pragma omp parallel num_threads(processors)
	// {
	// 	#pragma omp critical
	// 	{
	// 		int thread = omp_get_thread_num();
	// 		int threads = omp_get_num_threads();
	// 		cerr << getpid() << "\tProcess " << thread << " of " << threads << '/' << processors << endl;
	// 	}
	// }

	LArray keys( argv[1] );
	long processors;		//<	Number of threads to ask the parallel zone.
	long partitions;		//<	Number of partitions which will compose the array to sort.
	
	if (argc > 2)
	{
		partitions = strtol (argv[2], NULL, 0);
		processors = min (partitions, omp_get_num_procs ());
	}
	else
		partitions = processors = omp_get_num_procs ();


	long unsigned parts_sqr = partitions * partitions;
	long parts_lst = partitions - 1;
	long unsigned * psizes = new long unsigned[partitions];		//<	Size of each partition.
	long unsigned * pfirsts = new long unsigned[partitions];	//<	Offset of each partition.
	long unsigned * samp_ints = new long unsigned[parts_sqr];
	long * samples = new long[parts_sqr];
	long * pivots = new long[parts_lst];
	long unsigned * piv_ints = new long unsigned[partitions];
	long unsigned * sfirsts = new long unsigned[parts_sqr];
	long unsigned * ssizes = new long unsigned[parts_sqr];
	long * partfinal = new long[keys.length];


	//	CALCULATE PARTITIONS
	diveven (keys.length, partitions, psizes);

	
	pfirsts[0] = 0;
	for (long partition = 1 ; partition < partitions ; ++partition )
		pfirsts[partition] = pfirsts[partition-1] + psizes[partition-1];


	//	LOCAL SORT
	#pragma omp parallel for num_threads(processors) schedule(static)
	for (long partition = 0; partition < partitions; ++partition)
	{
		//int thread = omp_get_thread_num();
		// int threads = omp_get_num_threads();
		// #pragma omp critical
		// {
		// 	cerr << "Thread " << thread << " of " << threads << '/' << processors << endl << '\t';
		// 	for (long unsigned n = 0; n < psizes[thread]; ++n)
		// 		cerr << keys.data[pfirsts[thread] + n] << ' ';
		// 	cerr << endl;	
		// }
		qsort (keys.data + pfirsts[partition], psizes[partition], sizeof(long), comparel);
		// #pragma omp critical
		// {
		// 	cerr << "Thread " << thread << " of " << threads << '/' << processors << endl << '\t';
		// 	for (long unsigned n = 0; n < psizes[thread]; ++n)
		// 		cerr << keys.data[pfirsts[thread] + n] << ' ';
		// 	cerr << endl;	
		// }
	// }


	// //	SAMPLING
	// #pragma omp parallel for num_threads(processors) schedule(static)
	// for (long partition = 0; partition < partitions; ++partition)
	// {
		// int thread = omp_get_thread_num();
			// int threads = omp_get_num_threads();

		diveven (psizes[partition], partitions, samp_ints + partition * partitions);
			//debug
			// #pragma omp critical
			// {
			// 	cerr << "Thread " << thread << " of " << threads << '/' << processors << endl << '\t';
			// 	cerr << "Size: " << psizes[thread] << endl << '\t';
			// 	for (int n = 0; n < processors; ++n)
			// 		cerr << samp_ints[thread * processors + n] << ' ';
			// 	cerr << endl;	
			// }
		long unsigned w = 0;
		for (long p = 0; p < partitions; ++p)
		{
			samples[partition * partitions + p] = keys.data[pfirsts[partition] + w];
			w += samp_ints[partition * partitions + p];
		}
			// #pragma omp critical
			// {
			// 	cerr << "Thread " << thread << " of " << threads << '/' << processors << endl << '\t';
			// 	for (int n = 0; n < processors; ++n)
			// 		cerr << samples[thread * processors + n] << ' ';
			// 	cerr << endl;	
			// }
	}

	delete[] samp_ints;


	//	GATHER SAMPLES
		//debug
		// cerr << "Samples (merged): " << endl << '\t';
		// for (int n = 0; n < procs_sqr; ++n)
		// 	cerr << samples[n] << ' ';
		// cerr << endl;	

	//	ORDER SAMPLES
	qsort (samples, parts_sqr, sizeof(long), comparel);
		// cerr << "Samples (ordered): " << endl << '\t';
		// for (int n = 0; n < procs_sqr; ++n)
		// 	cerr << samples[n] << ' ';
		// cerr << endl;


	//	FIND PIVOTS
	//diveven (procs_sqr, processors, piv_ints);
		//debug
		// cerr << "Pivot intervals: " << endl << '\t';
		// for (int n = 0; n < procs_lst; ++n)
		// 	cerr << piv_ints[n] << ' ';
		// cerr << endl;
	{
		long unsigned w = 0;
		for (long p = 1; p < partitions; ++p)
		{
			pivots[p-1] = samples[w];
			w += partitions;
		}
	}
		//debug
		// cerr << "Pivots: " << endl << '\t';
		// for (long unsigned n = 0; n < procs_lst; ++n)
		// 	cerr << pivots[n] << ' ';
		// cerr << endl;

	delete[] piv_ints;


	//	SLICES
	#pragma omp parallel for num_threads(processors) schedule(static)
	for (long partition = 0; partition < partitions; ++partition)
	{
		// int thread = omp_get_thread_num ();
		int i = 0;
		long unsigned j = 0;
		long unsigned n = 0;
		long unsigned l = partition * partitions;

		sfirsts[l] = pfirsts[partition];
		while (n < psizes[partition])
		{
			if (i < parts_lst && keys.data[pfirsts[partition] + n] > pivots[i])
			{
				ssizes[l + i] = n - j;
				j = n;
				i += 1;
				sfirsts[l + i] = n + pfirsts[partition];
			}
			else
				n += 1;
		}
		ssizes[l + i] = n - j;
		for (i += 1; i < partitions; ++i)
		{
			sfirsts[l + i] = n + pfirsts[partition];;
			ssizes[l + i] = 0;
		}
			//debug
			// int threads = omp_get_num_threads();
			// #pragma omp critical
			// {
			// 	cerr << "Thread " << thread << " of " << threads << '/' << processors << endl << '\t';
			// 	for (int n = 0; n < processors; ++n)
			// 		cerr << sfirsts[l + n] << '/' << ssizes[l + n]<< ' ';
			// 	cerr << endl;	
			// }

			// #pragma omp barrier

			//debug
			// #pragma omp critical
			// {
			// 	cerr << "Thread " << thread << " of " << threads << '/' << processors << endl;
			// 	for (int s = 0; s < processors; ++s)
			// 	{
			// 		cerr << "\tSlice " << s << endl << "\t\t";
			// 		for (long unsigned n = 0; n < ssizes[l + s]; ++n)
			// 			cerr << keys.data[sfirsts[l + s] + n] << ' ';
			// 		cerr << endl;
			// 	}
			// 	cerr << endl;	
			// }
	}


	//	TRADE - SORT - GATHER
	#pragma omp parallel for num_threads(processors) schedule(static)
	for (long partition = 0; partition < partitions; ++partition)
	{
		// int thread = omp_get_thread_num ();

		psizes[partition] = 0;
		for (long p = 0; p < partitions; ++p)
			psizes[partition] += ssizes[p * partitions + partition];

			//debug
			// int threads = omp_get_num_threads();
			// #pragma omp critical
			// {
			// 	cerr << "Thread " << thread << " of " << threads << '/' << processors << endl << '\t' << psizes[thread] << endl;
			// 	cerr << endl;	
			// }
	}

	pfirsts[0] = 0;
	for (long p = 1; p < partitions; ++p)
		pfirsts[p] = pfirsts[p-1] + psizes[p-1];
		// //debug
		// cerr << "Final partitions:" << endl << '\t';
		// for (unsigned long r = 0; r < processors; ++r)
		// 	cerr << pfirsts[r] << '/' << psizes[r] << ' ';
		// cerr << endl;

	#pragma omp parallel for num_threads(processors) schedule(static)
	for (long partition = 0; partition < partitions; ++partition)
	{
		// int thread = omp_get_thread_num ();
		// int thread = t;
		long unsigned n = pfirsts[partition];
		for (long p = 0; p < partitions; ++p)
		{
			long unsigned i = p * partitions + partition;
			for (long unsigned s = 0; s < ssizes[i]; ++s)
				partfinal[n++] = keys.data[sfirsts[i] + s];
		}
			// //debug
			// int threads = omp_get_num_threads();
			// #pragma omp critical
			// {
			// 	cerr << "Thread " << thread << " of " << threads << '/' << processors << endl << '\t';
			// 	for (long unsigned n = 0; n < psizes[thread]; ++n)
			// 		cerr << partfinal[pfirsts[thread] + n] << ' ';
			// 	cerr << endl;
			// }
		qsort (partfinal + pfirsts[partition], psizes[partition], sizeof(long), comparel);
	}
	// cerr << "Result:" << endl << '\t';
	// for (long unsigned n = 0; n < keys.length; ++n)
	// 	cerr << partfinal[n] << ' ';
	// cerr << endl;

#ifdef	TK_PROFILE
	sw.stop();
	cout << sw.last().microseconds() << endl;
#else
	for (long unsigned n = 0; n < keys.length; ++n)
		cout << partfinal[n] << endl;
#endif

	delete[] partfinal;
	delete[] sfirsts;
	delete[] ssizes;
	delete[] pivots;
	delete[] samples;
	delete[] psizes;
	delete[] pfirsts;
	
	return 0;
}
