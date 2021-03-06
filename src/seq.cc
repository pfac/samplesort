#include <cerrno>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <cstdlib>

#include <unistd.h>

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
	if ( argc < 3 )
	{
		std::cerr
			<<	"Usage: "
			<<	argv[0]
			<<	" <int_array_filename> <processors>"
			<<	std::endl;
		throw(EINVAL);
	}

#ifdef	TK_PROFILE
	tk::Stopwatch sw;
	sw.start();
#endif

	LArray keys( argv[1] );
	long unsigned processors = strtoul( argv[2] , NULL , 0 );

	long unsigned * psizes = new long unsigned[processors];
	long unsigned * pfirsts = new long unsigned[processors];

	diveven (keys.length, processors, psizes);

	
	pfirsts[0] = 0;
	for (long unsigned r = 1 ; r < processors ; ++r )
		pfirsts[r] = pfirsts[r-1] + psizes[r-1];


	//	LOCAL SORT
	// #pragma omp parallel num_threads(processors)
	for (long unsigned thread = 0; thread < processors; ++thread)
	{
			//debug
			// cerr << "Thread " << thread << " of " << processors << endl << '\t';
			// for (long unsigned n = 0; n < psizes[thread]; ++n)
			// 	cerr << keys.data[pfirsts[thread] + n] << ' ';
			// cerr << endl;
		// int thread = omp_get_thread_num();
		qsort (keys.data + pfirsts[thread], psizes[thread], sizeof(long), comparel);
			//debug
			cerr << "Thread " << thread << " of " << processors << endl << '\t';
			for (long unsigned n = 0; n < psizes[thread]; ++n)
				cerr << keys.data[pfirsts[thread] + n] << ' ';
			cerr << endl;
	}


	//	SAMPLING
	long unsigned procs_sqr = processors * processors;
	long unsigned * samp_ints = new long unsigned[procs_sqr];
	long * samples = new long[procs_sqr];

	// #pragma omp parallel num_threads(processors)
	for (long unsigned thread = 0; thread < processors; ++thread)
	{
		// int thread = omp_get_thread_num();
		diveven (psizes[thread], processors, samp_ints + thread * processors);

			// //debug
			// cerr << "Thread " << thread << " of " << processors << endl << '\t';
			// cerr << "Size: " << psizes[thread] << endl << '\t';
			// for (int n = 0; n < processors; ++n)
			// 	cerr << samp_ints[thread * processors + n] << ' ';
			// cerr << endl;

		long unsigned w = 0;
		for (long unsigned r = 0; r < processors; ++r)
		{
			samples[thread * processors + r] = keys.data[pfirsts[thread] + w];
			w += samp_ints[thread * processors + r];
		}
			// debug
			cerr << "Thread " << thread << " of " << processors << endl << '\t';
			for (long unsigned n = 0; n < processors; ++n)
				cerr << samples[thread * processors + n] << ' ';
			cerr << endl;
	}

	delete[] samp_ints;


	//	GATHER SAMPLES
		// //debug
		// cerr << "Samples (merged): " << endl << '\t';
		// for (int n = 0; n < procs_sqr; ++n)
		// 	cerr << samples[n] << ' ';
		// cerr << endl;

	//	ORDER SAMPLES
	qsort (samples, procs_sqr, sizeof(long), comparel);
		// debug
		cerr << "Samples (ordered): " << endl << '\t';
		for (long unsigned n = 0; n < procs_sqr; ++n)
			cerr << samples[n] << ' ';
		cerr << endl;


	//	FIND PIVOTS
	long unsigned procs_lst = processors - 1;
	long * pivots = new long[procs_lst];
	long unsigned * piv_ints = new long unsigned[processors];

	diveven (procs_sqr, processors, piv_ints);
		// // debug
		// cerr << "Pivot intervals: " << endl << '\t';
		// for (int n = 0; n < procs_lst; ++n)
		// 	cerr << piv_ints[n] << ' ';
		// cerr << endl;
	{
		long unsigned w = piv_ints[0];
		for (long unsigned p = 1; p < processors; ++p)
		{
			pivots[p-1] = samples[w];
			w += piv_ints[p];
		}
	}
		// debug
		cerr << "Pivots: " << endl << '\t';
		for (long unsigned n = 0; n < procs_lst; ++n)
			cerr << pivots[n] << ' ';
		cerr << endl;

	delete[] piv_ints;


	//	SLICES
	long unsigned * sfirsts = new long unsigned[procs_sqr];
	long unsigned * ssizes = new long unsigned[procs_sqr];

	// #pragma omp parallel num_threads(processors)
	for (long unsigned thread = 0; thread < processors; ++thread)
	{
		long unsigned i = 0;
		long unsigned j = 0;
		long unsigned n;
		long unsigned l = thread * processors;

		sfirsts[l] = pfirsts[thread];
		i = 0;
		j = 0;
		n = 0;
		while (n < psizes[thread])
		{
			if (i < procs_lst && keys.data[pfirsts[thread] + n] > pivots[i])
			{
				ssizes[l + i] = n - j;
				j = n;
				i += 1;
				sfirsts[l + i] = n + pfirsts[thread];
			}
			else
				n += 1;
		}
		ssizes[l + i] = n - j;
		for (i += 1; i < processors; ++i)
		{
			sfirsts[l + i] = n + pfirsts[thread];
			ssizes[l + i] = 0;
		}

			// //debug
			// cerr << "Thread " << thread << " of " << processors << endl << '\t';
			// for (long unsigned n = 0; n < processors; ++n)
			// 	cerr << sfirsts[l + n] << '/' << ssizes[l + n]<< ' ';
			// cerr << endl;	

			// debug
			cerr << "Thread " << thread << " of " << processors << endl;
			for (long unsigned s = 0; s < processors; ++s)
			{
				cerr << "\tSlice " << s << endl << "\t\t";
				for (long unsigned n = 0; n < ssizes[l + s]; ++n)
					cerr << keys.data[sfirsts[l + s] + n] << ' ';
				cerr << endl;
			}
			cerr << endl;
	}


	// 	TRADE - SORT - GATHER
	long * partfinal = new long[keys.length];
	// #pragma omp parallel num_threads(processors)
	for (long unsigned thread = 0; thread < processors; ++thread)
	{
		psizes[thread] = 0;
		for (unsigned long r = 0; r < processors; ++r)
			psizes[thread] += ssizes[r * processors + thread];

			// debug
			cerr << "Thread " << thread << " of " << processors << endl << '\t' << psizes[thread] << endl;
			cerr << endl;	
	}

	pfirsts[0] = 0;
	for (unsigned long r = 1; r < processors; ++r)
		pfirsts[r] = pfirsts[r-1] + psizes[r-1];
		// debug
		cerr << "Final partitions:" << endl << '\t';
		for (unsigned long r = 0; r < processors; ++r)
			cerr << pfirsts[r] << '/' << psizes[r] << ' ';
		cerr << endl;

	// #pragma omp parallel num_threads(processors)
	for (long unsigned thread = 0; thread < processors; ++thread)
	{
		long unsigned n = pfirsts[thread];
		for (long unsigned r = 0; r < processors; ++r)
		{
			long unsigned i = r * processors + thread;
			for (long unsigned s = 0; s < ssizes[i]; ++s)
				partfinal[n++] = keys.data[sfirsts[i] + s];
		}

			// debug
			cerr << "Thread " << thread << " of " << processors << endl << '\t';
			for (long unsigned n = 0; n < psizes[thread]; ++n)
				cerr << partfinal[pfirsts[thread] + n] << ' ';
			cerr << endl;

		qsort (partfinal + pfirsts[thread], psizes[thread], sizeof(long), comparel);
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
