#pragma once
#ifndef ___CCUT_ARRAY_HPP___
#define ___CCUT_ARRAY_HPP___

#include <string>

namespace ccut
{



/*\************************************
 * LongArray
 *************************************/
	struct LArray
	{
		long unsigned length;
		long * data;

		LArray ( std::string filename );

		~LArray ();

		void print ( std::ostream& out , std::string delimiter ) const;
		void print ( std::ostream& out ) const;
		void print () const;

		long& operator[]( long unsigned i ) const;
	};



}

#include "ccut-array-inl.hpp"

#endif//___CCUT_ARRAY_HPP___
