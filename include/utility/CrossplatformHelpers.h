#pragma once

/// This is for Linux/Mac to avoid MSVC warning 4996 about non-check iterators
/// MSVC complains about using raw pointers with STL algorithms.
/// checked_array_iterator provides an iterator that ensures the bounds of the raw
/// array are not exceeded.
#ifndef _MSC_VER
namespace stdext
{
	template<typename T, typename SizeType>
	inline T checked_array_iterator(T arr, SizeType num_vals) { return arr; }
};
#endif
