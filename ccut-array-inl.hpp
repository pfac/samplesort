#pragma once
#ifndef	___CCUT_ARRAY_INL_HPP___
#define	___CCUT_ARRAY_INL_HPP___

#include "ccut-array.hpp"

#include <iostream>
#include <sstream>
using std::stringstream;

namespace ccut
{



/*\************************************
 * LongArray
 *************************************/
inline
LArray::~LArray ()
{ delete[] data; }



inline
void LArray::print ( std::ostream& out ) const
{
	stringstream ss;
	ss << std::endl;
	print(out,ss.str());
}



inline
void LArray::print () const
{
	stringstream ss;
	ss << std::endl;
	print(std::cout,ss.str());}



}//	namespace ccut

#endif//___CCUT_ARRAY_INL_HPP___
