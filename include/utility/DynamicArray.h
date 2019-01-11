#pragma once

#include "CrossplatformHelpers.h"

#include <assert.h>
#include <limits>
#include <vector>

namespace utility
{
	/// A dynamic array with very minimal functionality but also a small memory footprint.
	/// Note: specify SizeType such that the type can hold the longest size expected.
	template<typename T, typename SizeType>
	class DynamicArray
	{
	public:
		using value_type = T;
		using size_type = SizeType;

		DynamicArray();
		DynamicArray(const SizeType num_objects);
		/// A constructor that takes an iteratable container class that contains
		/// T objects.  It must have begin and end methods.
		/// Note: this will try to move the data from objects to this, though the type
		/// must support a move assignment operator (otherwise a copy will be made).
		template<typename TContainer>
		DynamicArray(TContainer&& objects);
		/// A constructor that takes an initializer list, use like:
		/// DynamicArray<int> int_array({ 1, 2 });
		/// DynamicArray<int*> int_p_array({ &int_1, &int_2 });
		/// Note: since the objects in an initializer list are const, this must copy them
		/// even though the initializer_list is an rvalue.
		/// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
		/// error since the unique_ptr will be const and not movable.
		/// Convert the initializer list to an array to use the constructor above.
		DynamicArray(std::initializer_list<T>&& objects);
		DynamicArray(const DynamicArray& arg);
		DynamicArray(DynamicArray&& arg) noexcept;
		virtual ~DynamicArray();

		/// Returns the beginning of the connectivity with type of
		/// const pointer to const pointers of objects.  Only the objects are multable.
		T * const begin() const;
		/// Returns the end of the connectivity with type of const pointer to const pointers of const objects.
		/// Note: this points just beyond the array.  DO NOT use any methods.  Only use to test iterator.
		T * const end() const;
		/// Returns the first node in the connectivity list
		T front() const;
		/// Returns the last node in the connectivity list
		T back() const;
		/// Returns the size of the container
		const SizeType& size() const;

		/// And an element to the end, reallocates for each addition (unlike vector)
		void push_back(const T& entry);
		/// Resizes the array to a certain number of objects.  The default constructor is called for each object.
		void resize(const SizeType num_objects);

		const T & operator [] (const SizeType i) const;
		T & operator [] (const SizeType i);
		/// Assign operator that takes an iterable lvalue container.
		/// Note: a copy is made for each object in objects.
		/// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
		/// error since the unique_ptr will be const and not movable.
		/// Convert the initializer list to an array to make the items moveable
		template<typename EntryPtrContainer>
		DynamicArray& operator = (const EntryPtrContainer &objects);
		/// Assign operator that takes an iterable rvalue container.
		/// Note: each object is moved to this container from objects.
		/// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
		/// error since the unique_ptr will be const and not movable.
		/// Convert the initializer list to an array to make the items moveable
		template<typename EntryPtrContainer>
		DynamicArray& operator = (EntryPtrContainer && objects);
		DynamicArray& operator = (const DynamicArray &other);

		/// Get a copy of the connectivity as a vector
		/// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
		/// error since the unique_ptr is not copyable.
		const std::vector<T> copy_as_vector() const;

		/// Clears the container and frees the memory
		void clear();

	protected:
		T* objects_;
		SizeType size_;
	};

	/////////////////
	// Definitions //
	/////////////////

	template<typename T, typename SizeType>
	inline DynamicArray<T, SizeType>::DynamicArray()
	{
		objects_ = nullptr;
		size_ = 0;
	}

	template<typename T, typename SizeType>
	inline DynamicArray<T, SizeType>::DynamicArray(const SizeType num_objects)
		: objects_(new T[num_objects]), size_(static_cast<SizeType>(num_objects))
	{
	}

	template<typename T, typename SizeType>
	template<typename TContainer>
	inline DynamicArray<T, SizeType>::DynamicArray(TContainer && objects)
		: objects_(new T[objects.size()]), size_(static_cast<SizeType>(objects.size()))
	{
		assert(objects.size() > 0);
		SizeType pos = 0;
		for (T& obj : objects)
		{
			objects_[pos] = std::move(obj);
			pos++;
		}
	}

	template<typename T, typename SizeType>
	inline DynamicArray<T, SizeType>::DynamicArray(std::initializer_list<T>&& objects)
		: objects_(new T[objects.size()]), size_(static_cast<SizeType>(objects.size()))
	{
		assert(objects.size() > 0);
		std::copy(objects.begin(), objects.end(), stdext::checked_array_iterator<T*>(objects_, size_));
	}

	template<typename T, typename SizeType>
	inline DynamicArray<T, SizeType>::DynamicArray(const DynamicArray& arg)
		: objects_(new T[arg.size_]), size_(arg.size_)
	{
		assert(arg.size() > 0);
		std::copy(&arg.objects_[0], &arg.objects_[arg.size_], stdext::checked_array_iterator<T*>(objects_, size_));
	}

	template<typename T, typename SizeType>
	inline DynamicArray<T, SizeType>::DynamicArray(DynamicArray && arg)
	{
		std::swap(objects_, arg.objects_);
		auto tmp_size = size_;
		size_ = arg.size_;
		arg.size_ = tmp_size;
	}

	template<typename T, typename SizeType>
	inline DynamicArray<T, SizeType>::~DynamicArray()
	{
		clear();
	}

	template<typename T, typename SizeType>
	inline T * const DynamicArray<T, SizeType>::begin() const
	{
		return &objects_[0];
	}

	template<typename T, typename SizeType>
	inline T * const DynamicArray<T, SizeType>::end() const
	{
		return &objects_[size_];
	}

	template<typename T, typename SizeType>
	inline T DynamicArray<T, SizeType>::front() const
	{
		return objects_[0];
	}

	template<typename T, typename SizeType>
	inline T DynamicArray<T, SizeType>::back() const
	{
		return objects_[size_ - 1];
	}

	template<typename T, typename SizeType>
	inline const SizeType & DynamicArray<T, SizeType>::size() const
	{
		return size_;
	}

	template<typename T, typename SizeType>
	inline void DynamicArray<T, SizeType>::push_back(const T & entry)
	{
		auto tmp = objects_;
		objects_ = new T[size_ + 1];
		std::copy(&tmp[0], &tmp[size_], stdext::checked_array_iterator<T*>(objects_, size_));
		objects_[size_] = T(entry);
		size_++;
		delete[] tmp;
	}

	template<typename T, typename SizeType>
	inline void DynamicArray<T, SizeType>::resize(const SizeType num_objects)
	{
		if (objects_ != nullptr)
			delete[] objects_;

		objects_ = new T[num_objects]();
		size_ = num_objects;
	}

	template<typename T, typename SizeType>
	inline const T & DynamicArray<T, SizeType>::operator[](const SizeType i) const
	{
		assert(i >= 0);
		assert(i < size_);
		return objects_[i];
	}

	template<typename T, typename SizeType>
	inline T& DynamicArray<T, SizeType>::operator[](const SizeType i)
	{
		assert(i >= 0);
		assert(i < size_);
		return objects_[i];
	}

	template<typename T, typename SizeType>
	template<typename EntryPtrContainer>
	inline DynamicArray<T, SizeType> & DynamicArray<T, SizeType>::operator=(const EntryPtrContainer & objects)
	{
		assert(objects.size() < std::numeric_limits<SizeType>::max());
		delete[] this->objects_;
		this->objects_ = new T[objects.size()];
		this->size_ = static_cast<SizeType>(objects.size());
		std::copy(objects.begin(), objects.end(), stdext::checked_array_iterator<T*>(this->objects_, this->size_));
		return *this;
	}

	template<typename T, typename SizeType>
	template<typename EntryPtrContainer>
	inline DynamicArray<T, SizeType> & DynamicArray<T, SizeType>::operator=(EntryPtrContainer && objects)
	{
		objects_ = new T[objects.size()];
		size_ = static_cast<SizeType>(objects.size());
		SizeType pos = 0;
		for (T& obj : objects)
		{
			objects_[pos] = std::move(obj);
			pos++;
		}
		return *this;
	}

	template<typename T, typename SizeType>
	inline DynamicArray<T, SizeType> & DynamicArray<T, SizeType>::operator=(const DynamicArray & other)
	{
		delete[] this->objects_;
		this->objects_ = new T[other.size_];
		this->size_ = other.size_;
		std::copy(&other.objects_[0], &other.objects_[other.size_], stdext::checked_array_iterator<T*>(this->objects_, this->size_));
		return *this;
	}

	template<typename T, typename SizeType>
	inline const std::vector<T> DynamicArray<T, SizeType>::copy_as_vector() const
	{
		std::vector<T> conn_copy(size());
		std::copy(&objects_[0], &objects_[size_], conn_copy.begin());
		return conn_copy;
	}

	template<typename T, typename SizeType>
	inline void DynamicArray<T, SizeType>::clear()
	{
		delete[] objects_;
		size_ = 0;
	}

}
