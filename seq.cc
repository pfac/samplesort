#include "ccut-array.hpp"

#include <cerrno>
#include <iostream>

#include "psrs.hpp"

using ccut::LArray;

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

	LArray keys( argv[1] );
	long unsigned nprocessors = strtoul( argv[2] , NULL , 0 );

	psrs( keys.data , keys.length , nprocessors );

	return 0;
}
