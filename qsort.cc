#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <errno.h>

#include "ccut-array.hpp"
using ccut::LArray;

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

	LArray keys( argv[1] );
	qsort (keys.data, keys.length, sizeof(long), comparel);

	for (long unsigned n = 0; n < keys.length; ++n)
		cout << keys.data[n] << endl;

	return 0;
}