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
				// cerr << "Partition " << p << " of " << processors << endl << '\t';
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
			<<	" <int_array_filename> <partitions>"
			<<	std::endl;
		throw(EINVAL);
	}

#ifdef	TK_PROFILE
	tk::Stopwatch sw;
	sw.start();
#endif

	LArray keys( argv[1] );
	long unsigned processors = strtoul( argv[2] , NULL , 0 );

	//	Initialize task scheduler
	tbb::task_scheduler_init init(processors);
		// // debug
		// tbb::task_scheduler_init init(1);

	long unsigned * psizes = new long unsigned[processors];
	long unsigned * pfirsts = new long unsigned[processors];

	roundrobin (keys.length, processors, psizes);

	pfirsts[0] = 0;
	for (long unsigned r = 1 ; r < processors ; ++r )
		pfirsts[r] = pfirsts[r-1] + psizes[r-1];


	//	LOCAL SORT
	TBB_Sorter sorter;
	sorter.data = keys.data;
	sorter.partitions.count = processors;
	sorter.partitions.firsts = pfirsts;
	sorter.partitions.sizes = psizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, processors, 1), sorter);
		// // debug
		// for (long unsigned p = 0; p < processors; ++p) {
		// 	cerr << "Partition " << p << " of " << processors << endl;
		// 	if (psizes[p]) {
		// 		cerr << '\t' << keys.data[pfirsts[p]];
		// 		for (long unsigned e = 1; e < psizes[p]; ++e) {
		// 			cerr << ' ' << keys.data[pfirsts[p] + e];
		// 		}
		// 		cerr << endl;
		// 	}
		// }


	//	SAMPLING
	long unsigned procs_sqr = processors * processors;
	long unsigned * samp_ints = new long unsigned[procs_sqr];
	long * samples = new long[procs_sqr];

	// // #pragma omp parallel num_threads(processors)
	// for (long unsigned thread = 0; thread < processors; ++thread)
	// {
	// 	// int thread = omp_get_thread_num();
	// 	roundrobin (psizes[thread], processors, samp_ints + thread * processors);

	// 		// //debug
	// 		// cerr << "Thread " << thread << " of " << processors << endl << '\t';
	// 		// cerr << "Size: " << psizes[thread] << endl << '\t';
	// 		// for (int n = 0; n < processors; ++n)
	// 		// 	cerr << samp_ints[thread * processors + n] << ' ';
	// 		// cerr << endl;

	// 	long unsigned w = 0;
	// 	for (long unsigned r = 0; r < processors; ++r)
	// 	{
	// 		samples[thread * processors + r] = keys.data[pfirsts[thread] + w];
	// 		w += samp_ints[thread * processors + r];
	// 	}
	// 		//debug
	// 		// cerr << "Thread " << thread << " of " << processors << endl << '\t';
	// 		// for (int n = 0; n < processors; ++n)
	// 		// 	cerr << samples[thread * processors + n] << ' ';
	// 		// cerr << endl;
	// }

	TBB_Sampler sampler;
	sampler.data = keys.data;
	sampler.samples = samples;
	sampler.intervals = samp_ints;
	sampler.partitions.count = processors;
	sampler.partitions.firsts = pfirsts;
	sampler.partitions.sizes = psizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, processors, 1), sampler);
		// // debug
		// cerr << "SAMPLES" << endl;
		// for (long unsigned p = 0; p < processors; ++p) {
		// 	cerr << "Partition " << p << " of " << processors << endl;
		// 	cerr << '\t' << samples[p * processors];
		// 	for (long unsigned q = 1; q < processors; ++q)
		// 		cerr << ' ' << samples[p * processors + q];
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
	long unsigned procs_lst = processors - 1;
	long * pivots = new long[procs_lst];
	long unsigned * piv_ints = new long unsigned[processors];

	roundrobin (procs_sqr, processors, piv_ints);
		//debug
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

	// // #pragma omp parallel num_threads(processors)
	// for (long unsigned thread = 0; thread < processors; ++thread)
	// {
	// 	long unsigned i = 0;
	// 	long unsigned j = 0;
	// 	long unsigned n;
	// 	long unsigned l = thread * processors;

	// 	sfirsts[l] = pfirsts[thread];
	// 	i = 0;
	// 	j = 0;
	// 	n = 0;
	// 	while (n < psizes[thread])
	// 	{
	// 		if (i < procs_lst && keys.data[pfirsts[thread] + n] > pivots[i])
	// 		{
	// 			ssizes[l + i] = n - j;
	// 			j = n;
	// 			i += 1;
	// 			sfirsts[l + i] = n + pfirsts[thread];
	// 		}
	// 		else
	// 			n += 1;
	// 	}
	// 	ssizes[l + i] = n - j;
	// 	for (i += 1; i < processors; ++i)
	// 	{
	// 		sfirsts[l + i] = n + pfirsts[thread];
	// 		ssizes[l + i] = 0;
	// 	}

	// 		// //debug
	// 		// cerr << "Thread " << thread << " of " << processors << endl << '\t';
	// 		// for (long unsigned n = 0; n < processors; ++n)
	// 		// 	cerr << sfirsts[l + n] << '/' << ssizes[l + n]<< ' ';
	// 		// cerr << endl;	

	// 		// debug
	// 		cerr << "Thread " << thread << " of " << processors << endl;
	// 		for (long unsigned s = 0; s < processors; ++s)
	// 		{
	// 			cerr << "\tSlice " << s << endl << "\t\t";
	// 			for (long unsigned n = 0; n < ssizes[l + s]; ++n)
	// 				cerr << keys.data[sfirsts[l + s] + n] << ' ';
	// 			cerr << endl;
	// 		}
	// 		cerr << endl;
	// }

	TBB_Slicer slicer;
	slicer.data = keys.data;
	slicer.pivots = pivots;
	slicer.partitions.count = processors;
	slicer.partitions.last = procs_lst;
	slicer.partitions.firsts = pfirsts;
	slicer.partitions.sizes = psizes;
	slicer.slices.firsts = sfirsts;
	slicer.slices.sizes = ssizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, processors, 1), slicer);


	// 	TRADE - SORT - GATHER
	long * partfinal = new long[keys.length];

	// #pragma omp parallel num_threads(processors)
	// for (long unsigned thread = 0; thread < processors; ++thread)
	// {
	// 	psizes[thread] = 0;
	// 	for (unsigned long r = 0; r < processors; ++r)
	// 		psizes[thread] += ssizes[r * processors + thread];

	// 		//debug
	// 		// cerr << "Thread " << thread << " of " << processors << endl << '\t' << psizes[thread] << endl;
	// 		// cerr << endl;	
	// }
	TBB_Trader trader;
	trader.partitions.count = processors;
	trader.partitions.sizes = psizes;
	trader.slices.sizes = ssizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, processors, 1), trader);

	pfirsts[0] = 0;
	for (unsigned long r = 1; r < processors; ++r)
		pfirsts[r] = pfirsts[r-1] + psizes[r-1];
		// //debug
		// cerr << "Final partitions:" << endl << '\t';
		// for (unsigned long r = 0; r < processors; ++r)
		// 	cerr << pfirsts[r] << '/' << psizes[r] << ' ';
		// cerr << endl;

	// #pragma omp parallel num_threads(processors)
	// for (long unsigned thread = 0; thread < processors; ++thread)
	// {
	// 	long unsigned n = pfirsts[thread];
	// 	for (long unsigned r = 0; r < processors; ++r)
	// 	{
	// 		long unsigned i = r * processors + thread;
	// 		for (long unsigned s = 0; s < ssizes[i]; ++s)
	// 			partfinal[n++] = keys.data[sfirsts[i] + s];
	// 	}

	// 		//debug
	// 		// cerr << "Thread " << thread << " of " << processors << endl << '\t';
	// 		// for (long unsigned n = 0; n < psizes[thread]; ++n)
	// 		// 	cerr << partfinal[pfirsts[thread] + n] << ' ';
	// 		// cerr << endl;

	// 	qsort (partfinal + pfirsts[thread], psizes[thread], sizeof(long), comparel);
	// }
	// cerr << "Result:" << endl << '\t';
	// for (long unsigned n = 0; n < keys.length; ++n)
	// 	cerr << partfinal[n] << ' ';
	// cerr << endl;
	TBB_MergerSorter mergesorter;
	mergesorter.data.unsorted = keys.data;
	mergesorter.data.sorted = partfinal;
	mergesorter.partitions.count = processors;
	mergesorter.partitions.firsts = pfirsts;
	mergesorter.partitions.sizes = psizes;
	mergesorter.slices.firsts = sfirsts;
	mergesorter.slices.sizes = ssizes;
	parallel_for(tbb::blocked_range<long unsigned>(0, processors, 1), mergesorter);

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
