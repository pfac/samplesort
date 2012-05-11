#include "psrs.hpp"

#include "ccut-util.hpp"



void quicksort(long arr[], long unsigned left, long unsigned right) {
      long unsigned i = left, j = right;
      long tmp;
      long pivot = arr[(left + right) / 2];
 
      /* partition */
      while (i <= j) {
            while (arr[i] < pivot)
                  i++;
            while (arr[j] > pivot)
                  j--;
            if (i <= j) {
                  tmp = arr[i];
                  arr[i] = arr[j];
                  arr[j] = tmp;
                  i++;
                  j--;
            }
      };
 
      /* recursion */
      if (left < j)
            quicksort(arr, left, j);
      if (i < right)
            quicksort(arr, i, right);
}



int comparel (const void * a, const void * b)
{
  return ( *(long*)a - *(long*)b );
}


void psrs ( long keys[] , long unsigned nkeys , long unsigned nprocessors )
{
	long unsigned n;
	long unsigned p;
		//debug
		//printf( "%lu\n" , nkeys );
		//for ( n = 0 ; n < nkeys ; ++n )
		//	std::cerr << keys[n] << std::endl;
		//printf( "%lu\n" , nprocessors );


	//	1.	PARTITION
	long unsigned * pfirsts = new long unsigned[nprocessors];
	long unsigned * psizes = new long unsigned[nprocessors];

	divevenUL( nkeys , nprocessors , psizes );
		//debug
		//for ( p = 0 ; p < nprocessors ; ++p )
		//	std::cerr << psizes[p] << std::endl;

	pfirsts[0] = 0;
	for ( p = 1 ; p < nprocessors ; ++p )
		pfirsts[p] = pfirsts[p-1] + psizes[p-1];
		//debug
		for ( p = 0 ; p < nprocessors ; ++p )
		{
			std::cerr << "Partition " << p << ':' << std::endl << '\t';
			for ( n = 0 ; n < psizes[p] ; ++n )
				std::cerr << (keys + pfirsts[p])[n] << ' ';
			std::cerr << std::endl;
		}
	

	//	2.	LOCAL SORT
	for ( p = 0 ; p < nprocessors ; ++p )
		//quicksort( keys + pfirsts[p] , psizes[p] );
		//quicksort( keys , pfirsts[p] , pfirsts[p] + psizes[p] - 1 );
		qsort( keys + pfirsts[p] , psizes[p] , sizeof(long) , comparel );
		//debug
		for ( p = 0 ; p < nprocessors ; ++p )
		{
			std::cerr << "Partition " << p << ':' << std::endl << '\t';
			for ( n = 0 ; n < psizes[p] ; ++n )
				std::cerr << (keys + pfirsts[p])[n] << ' ';
			std::cerr << std::endl;
		}
	
	

	//	3.	REGULAR SAMPLES
	//		Each processor collects p samples
	long unsigned nprocssqr = nprocessors * nprocessors;
	long unsigned sampregint = nkeys / nprocssqr;
	long * samples = new long[nprocssqr];
	long unsigned s;

	for ( p = 0 ; p < nprocessors ; ++p )
		for ( s = 0 ; s < nprocessors ; ++s )
			samples[ p * nprocessors + s ] = keys[ pfirsts[p] + s * sampregint ];
			//debug
			for ( n = 0 ; n < nprocssqr ; ++n )
				std::cerr << samples[n] << ' ';
			std::cerr << std::endl;
	

	//	4.	ORDER SAMPLES
	qsort( samples , nprocssqr , sizeof(long) , comparel );
		//debug
		for ( n = 0 ; n < nprocssqr ; ++n )
			std::cerr << samples[n] << ' ';
		std::cerr << std::endl;
	

	//	5.	SELECT PIVOTS
	long unsigned npivots = nprocessors - 1;
	long * pivots = new long[npivots];
	long unsigned nprocshlf = nprocessors / 2;

	for ( p = 1 ; p < nprocessors ; ++p )
	{
		long unsigned index = p * nprocessors + nprocshlf - 1;
		pivots[p-1] = samples[index];
	}
		//debug
		for ( n = 0 ; n < npivots ; ++n )
			std::cerr << pivots[n] << ' ';
		std::cerr << std::endl;

	
	//	6.	SLICES
	long * slices = new long[ nprocssqr * psizes[0] ];
	long unsigned * ssizes = new long unsigned[nprocssqr];

	for ( p = 0 ; p < nprocessors ; ++p )
	{
		long unsigned i = 0;
		long unsigned j = 0;
		long * partition = keys + pfirsts[p];
		for ( n = 0 ; n < psizes[p] ; ++n )
		{
			if ( partition[n] > pivots[i] )
			{
				ssizes[ p * nprocessors + i ] = j + 1;
				i += 1;
				j = 0;
			}
			slices[ p * nprocessors * psizes[p] + i * psizes[p] + j++ ] = partition[n];
		}
	}
		//debug
		for ( p = 0 ; p < nprocessors ; ++p )
		{
			std::cerr << "Partition " << p << ':' << std::endl << '\t';
			for ( s = 0 ; s < nprocessors ; ++s )
			{
				std::cerr << "\tSlice " << s << ':' << std::endl << "\t\t";
				for ( n = 0 ; n < ssizes[ p * nprocessors + s ] ; ++n )
					std::cerr << slices[ p * nprocessors * psizes[p] + s * psizes[p] + n ] << ' ';
				std::cerr << std::endl;
			}
			std::cerr << std::endl;
		}

	
	delete[] samples;

	delete[] pfirsts;
	delete[] psizes;
}

