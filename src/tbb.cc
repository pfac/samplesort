#include <cerrno>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <cstdlib>

#include <unistd.h>

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include "ccut-array.hpp"
using ccut::LArray;

#ifdef	TK_PROFILE
#include "tk/stopwatch.hpp"
#endif



void roundrobin ( long unsigned x , long unsigned y , long unsigned a[] )
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





struct TBB_Sorter {
	long * data;
	struct {
		long unsigned count;
		long unsigned * firsts;
		long unsigned * sizes;
	} partitions;

	void operator() (const tbb::blocked_range<long unsigned> &range) const {
		for (long unsigned p = range.begin(); p < range.end(); ++p) {
			qsort(data + partitions.firsts[p], partitions.sizes[p], sizeof(long), comparel);
				// // debug
				// cerr << "Partition " << p << " of " << partitions.count << endl << '\t';
				// for (long unsigned n = 0; n < partitions.sizes[p]; ++n)
				// 	cerr << data[partitions.firsts[p] + n] << ' ';
				// cerr << endl;
		}
	}
};

struct TBB_Sampler {
	long * data;
	long * samples;
	long unsigned * intervals;
	struct {
		long unsigned count;
		long unsigned * firsts;
		long unsigned * sizes;
	} partitions;

	void operator() (const tbb::blocked_range<long unsigned> &range) const {
		for (long unsigned p = range.begin(); p < range.end(); ++p) {
			roundrobin(partitions.sizes[p], partitions.count, intervals + p * partitions.count);			
				// // debug
				// cerr << "Partition " << p << " of " << partitions.count << endl;
				// if (partitions.count) {
				// 	cerr << '\t' << intervals[p * partitions.count];
				// 	for (long unsigned n = 1; n < partitions.count; ++n)
				// 		cerr << ' ' << intervals[p * partitions.count + n];
				// }
				// cerr << endl;

			long unsigned w = 0;
			for (long unsigned r = 0; r < partitions.count; ++r) {
				samples[p * partitions.count + r] = data[partitions.firsts[p] + w];
				w += intervals[p * partitions.count + r];
			}
				// // debug
				// cerr << "Partition " << p << " of " << partitions.count << endl;
				// if (partitions.count) {
				// 	cerr << '\t' << samples[p * partitions.count];
				// 	for (long unsigned q = 1; q < partitions.count; ++q)
				// 		cerr << ' ' << samples[p * partitions.count + q];
				// 	cerr << endl;
				// }
		}
	}
};

struct TBB_Slicer {
	long * data;
	long * pivots;
	struct {
		long unsigned count;
		long unsigned last;
		long unsigned * firsts;
		long unsigned * sizes;
	} partitions;
	struct {
		long unsigned * firsts;
		long unsigned * sizes;
	} slices;

	void operator() (const tbb::blocked_range<long unsigned> &range) const {
		for (long unsigned p = range.begin(); p < range.end(); ++p) {
			long unsigned i = 0;
			long unsigned j = 0;
			long unsigned n = 0;
			long unsigned l = p * partitions.count;

			slices.firsts[l] = partitions.firsts[p];
			while (n < partitions.sizes[p])
			{
				if (i < partitions.last && data[partitions.firsts[p] + n] > pivots[i])
				{
					slices.sizes[l + i] = n - j;
					j = n;
					i += 1;
					slices.firsts[l + i] = n + partitions.firsts[p];
				}
				else
					n += 1;
			}
			slices.sizes[l + i] = n - j;
			for (i += 1; i < partitions.count; ++i)
			{
				slices.firsts[l + i] = n + partitions.firsts[p];
				slices.sizes[l + i] = 0;
			}
				// //debug
				// cerr << "Partition " << p << " of " << partitions.count << endl << '\t';
				// for (long unsigned n = 0; n < partitions.count; ++n)
				// 	cerr << slices.firsts[l + n] << '/' << slices.sizes[l + n] << ' ';
				// cerr << endl;	

				// // debug
				// cerr << "Partition " << p << " of " << partitions.count << endl;
				// for (long unsigned s = 0; s < partitions.count; ++s)
				// {
				// 	cerr << "\tSlice " << s << endl << "\t\t";
				// 	for (long unsigned n = 0; n < slices.sizes[l + s]; ++n)
				// 		cerr << data[slices.firsts[l + s] + n] << ' ';
				// 	cerr << endl;
				// }
				// cerr << endl;
		}
	}
};

struct TBB_Trader {
	struct {
		long unsigned count;
		long unsigned * sizes;
	} partitions;
	struct {
		long unsigned * sizes;
	} slices;

	void operator() (const tbb::blocked_range<long unsigned> &range) const {
		for (long unsigned p = range.begin(); p < range.end(); ++p) {
			partitions.sizes[p] = 0;
			for (unsigned long s = 0; s < partitions.count; ++s)
				partitions.sizes[p] += slices.sizes[s * partitions.count + p];
		}
	}
};

struct TBB_MergerSorter {
	struct {
		long * unsorted;
		long * sorted;
	} data;
	struct {
		long unsigned count;
		long unsigned * firsts;
		long unsigned * sizes;
	} partitions;
	struct {
		long unsigned * firsts;
		long unsigned * sizes;
	} slices;

	void operator() (const tbb::blocked_range<long unsigned> &range) const {
		for (long unsigned p = range.begin(); p < range.end(); ++p) {
			long unsigned n = partitions.firsts[p];
			for (long unsigned r = 0; r < partitions.count; ++r)
			{
				long unsigned i = r * partitions.count + p;
				for (long unsigned s = 0; s < slices.sizes[i]; ++s)
					data.sorted[n++] = data.unsorted[slices.firsts[i] + s];
			}

				// // debug
				// cerr << "Partition " << p << " of " << partitions.count << endl << '\t';
				// for (long unsigned n = 0; n < psizes[thread]; ++n)
				// 	cerr << partfinal[pfirsts[thread] + n] << ' ';
				// cerr << endl;

			qsort (data.sorted + partitions.firsts[p], partitions.sizes[p], sizeof(long), comparel);
		}
	}
};







int main ( int argc , char *argv[] )
{
	if ( argc < 3 )
	{
		std::cerr
			<<	"Usage: "
			<<	argv[0]
			<<	" <int_array_filename> <partitions> [<threads>]"
			<<	std::endl;
		throw(EINVAL);
	}

#ifdef	TK_PROFILE
	tk::Stopwatch sw;
	sw.start();
#endif

	LArray keys( argv[1] );
	long unsigned partitions = strtoul( argv[2] , NULL , 0 );

	//	Initialize task scheduler
	if (argc > 3) {
		long unsigned threads = strtoul(argv[3], NULL, 0);
		tbb::task_scheduler_init init(threads);
	} else
		tbb::task_scheduler_init init;
	
		// // debug
		// tbb::task_scheduler_init init(1);

	// long unsigned grainsize = ceil((double) partitions / (double) threads);
	long unsigned grainsize = 1;

	long unsigned * psizes = new long unsigned[partitions];
	long unsigned * pfirsts = new long unsigned[partitions];

	roundrobin (keys.length, partitions, psizes);

	pfirsts[0] = 0;
	for (long unsigned r = 1 ; r < partitions ; ++r )
		pfirsts[r] = pfirsts[r-1] + psizes[r-1];


	//	LOCAL SORT
	TBB_Sorter sorter;
	sorter.data = keys.data;
	sorter.partitions.count = partitions;
	sorter.partitions.firsts = pfirsts;
	sorter.partitions.sizes = psizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, partitions, grainsize), sorter);
		// // debug
		// for (long unsigned p = 0; p < partitions; ++p) {
		// 	cerr << "Partition " << p << " of " << partitions << endl;
		// 	if (psizes[p]) {
		// 		cerr << '\t' << keys.data[pfirsts[p]];
		// 		for (long unsigned e = 1; e < psizes[p]; ++e) {
		// 			cerr << ' ' << keys.data[pfirsts[p] + e];
		// 		}
		// 		cerr << endl;
		// 	}
		// }


	//	SAMPLING
	long unsigned procs_sqr = partitions * partitions;
	long unsigned * samp_ints = new long unsigned[procs_sqr];
	long * samples = new long[procs_sqr];

	TBB_Sampler sampler;
	sampler.data = keys.data;
	sampler.samples = samples;
	sampler.intervals = samp_ints;
	sampler.partitions.count = partitions;
	sampler.partitions.firsts = pfirsts;
	sampler.partitions.sizes = psizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, partitions, grainsize), sampler);
		// // debug
		// cerr << "SAMPLES" << endl;
		// for (long unsigned p = 0; p < partitions; ++p) {
		// 	cerr << "Partition " << p << " of " << partitions << endl;
		// 	cerr << '\t' << samples[p * partitions];
		// 	for (long unsigned q = 1; q < partitions; ++q)
		// 		cerr << ' ' << samples[p * partitions + q];
		// 	cerr << endl;
		// }

	delete[] samp_ints;


	//	GATHER SAMPLES
		// //debug
		// cerr << "Samples (merged): " << endl << '\t';
		// for (int n = 0; n < procs_sqr; ++n)
		// 	cerr << samples[n] << ' ';
		// cerr << endl;

	//	ORDER SAMPLES
	qsort (samples, procs_sqr, sizeof(long), comparel);
		//debug
		// cerr << "Samples (ordered): " << endl << '\t';
		// for (int n = 0; n < procs_sqr; ++n)
		// 	cerr << samples[n] << ' ';
		// cerr << endl;


	//	FIND PIVOTS
	long unsigned procs_lst = partitions - 1;
	long * pivots = new long[procs_lst];
	long unsigned * piv_ints = new long unsigned[partitions];

	roundrobin (procs_sqr, partitions, piv_ints);
		//debug
		// cerr << "Pivot intervals: " << endl << '\t';
		// for (int n = 0; n < procs_lst; ++n)
		// 	cerr << piv_ints[n] << ' ';
		// cerr << endl;
	{
		long unsigned w = piv_ints[0];
		for (long unsigned p = 1; p < partitions; ++p)
		{
			pivots[p-1] = samples[w];
			w += piv_ints[p];
		}
	}
		// // debug
		// cerr << endl << "PIVOTS" << endl;
		// cerr << "Pivots: " << endl << '\t';
		// for (long unsigned n = 0; n < procs_lst; ++n)
		// 	cerr << pivots[n] << ' ';
		// cerr << endl;

	delete[] piv_ints;


	//	SLICES
	long unsigned * sfirsts = new long unsigned[procs_sqr];
	long unsigned * ssizes = new long unsigned[procs_sqr];

	TBB_Slicer slicer;
	slicer.data = keys.data;
	slicer.pivots = pivots;
	slicer.partitions.count = partitions;
	slicer.partitions.last = procs_lst;
	slicer.partitions.firsts = pfirsts;
	slicer.partitions.sizes = psizes;
	slicer.slices.firsts = sfirsts;
	slicer.slices.sizes = ssizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, partitions, grainsize), slicer);


	// 	TRADE - SORT - GATHER
	long * partfinal = new long[keys.length];

	TBB_Trader trader;
	trader.partitions.count = partitions;
	trader.partitions.sizes = psizes;
	trader.slices.sizes = ssizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, partitions, grainsize), trader);

	pfirsts[0] = 0;
	for (unsigned long r = 1; r < partitions; ++r)
		pfirsts[r] = pfirsts[r-1] + psizes[r-1];
		// //debug
		// cerr << "Final partitions:" << endl << '\t';
		// for (unsigned long r = 0; r < partitions; ++r)
		// 	cerr << pfirsts[r] << '/' << psizes[r] << ' ';
		// cerr << endl;

	TBB_MergerSorter mergesorter;
	mergesorter.data.unsorted = keys.data;
	mergesorter.data.sorted = partfinal;
	mergesorter.partitions.count = partitions;
	mergesorter.partitions.firsts = pfirsts;
	mergesorter.partitions.sizes = psizes;
	mergesorter.slices.firsts = sfirsts;
	mergesorter.slices.sizes = ssizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, partitions, grainsize), mergesorter);

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
