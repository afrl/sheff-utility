#pragma once

#include <algorithm>
#include <vector>

namespace utility
{
	/// A wrapper for the std::vector that inserts values only if they do not exist.
	/// It is a set-like vector.  However, some optimization is needed to know if it
	/// is better to sort as values are inserted and use the binary_search to see if
	/// an item already exists.  This class is generally used for computing sparsity
	/// patters, so the alternate methodology may provide better performance for that
	/// particular use case.
	template <class T>
	class VectorSet
	{

	public:
		VectorSet() {};
		~VectorSet() {};

		/// Returns the size of the container (the number of items stored)
		inline const std::size_t size() const { return v.size(); }
		/// Tries to inserts an item into the container and returns a bool
		/// indicating if an item was actually inserted.
		bool insert(const T& val);
		/// Sorts the container's items based on value and returns
		/// a const reference to the sorted vector.
		const std::vector<T>& get_sorted();
		auto begin() const { return v.begin(); }
		auto end() const { return v.end(); }


	private:
		std::vector<T> v;
	};

	template<class T>
	inline bool VectorSet<T>::insert(const T & val)
	{
		for (const T& i : v)
		{
			if (val == i)
			{
				return false;
			}
		}
		v.push_back(val);
		return true;
	}

	template<class T>
	inline const std::vector<T>& VectorSet<T>::get_sorted()
	{
		std::sort(v.begin(), v.end());
		return v;
	}
}