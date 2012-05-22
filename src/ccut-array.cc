#include "ccut-array.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace ccut
{



/*\************************************
 * LongArray
 *************************************/
LArray::LArray ( std::string filename )
{
	long unsigned rows;
	long unsigned cols;
	std::ifstream file( filename.c_str() );

	if ( file.is_open() )
	{
		file
			>>	rows
			>>	cols
			;
		length = rows * cols;
		data = new long[length];
		for ( long unsigned i = 0 ; i < length ; ++i )
			file
				>>	data[i];
		file.close();
	}
	else
		std::cerr
			<< "Error opening file " << filename << std::endl;
}

void LArray::print ( std::ostream& out , std::string delimiter ) const
{
	for ( long unsigned i = 0 ; i < length ; ++i )
		out
			<<	data[i]
			<<	delimiter;
}

}//	namespace ccut
