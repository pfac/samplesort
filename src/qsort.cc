#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <errno.h>
#include <cstdlib>

#include "ccut-array.hpp"
using ccut::LArray;

#ifdef	TK_PROFILE
#include "tk/stopwatch.hpp"
#endif

int comparel (const void * a, const void * b)
{
  return ( *(long*)a - *(long*)b );
}

int main (int argc, char *argv[])
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

#ifdef	TK_PROFILE
	tk::Stopwatch sw;
	sw.start();
#endif

	LArray keys( argv[1] );
	qsort (keys.data, keys.length, sizeof(long), comparel);

#ifdef	TK_PROFILE
	sw.stop();
	cout << sw.last().microseconds() << endl;
#else
	for (long unsigned n = 0; n < keys.length; ++n)
		cout << keys.data[n] << endl;
#endif

	return 0;
}
