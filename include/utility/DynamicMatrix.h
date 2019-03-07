#pragma once

#include "CrossplatformHelpers.h"

#include <assert.h>
#include <limits>
#include <vector>

namespace utility
{
	template<typename T, typename ColSizeType>
	class DynamicMatrixRow
	{
	public:
		DynamicMatrixRow(T* begin_a, T* end_a)
			: begin_(begin_a), end_(end_a)
		{}

		T * const begin() const { return begin_; }
		T * const end() const { return end_; }
        const ColSizeType size() const { return static_cast<ColSizeType>(end_ - begin_); }
        T & operator[](const ColSizeType i) const { return *(begin_ + i); }

	private:
		T* begin_;
		T* end_;
	};

	/// A dynamic matrix (square) with very minimal functionality but also a small memory footprint.
	/// Note: specify RowSizeType and ColSizeType such that the type can hold the longest size expected.
	template<typename T, typename RowSizeType, typename ColSizeType>
	class DynamicMatrix
	{
	public:
		using value_type = T;
		using row_size_type = RowSizeType;
		using col_size_type = ColSizeType;

		DynamicMatrix();
		DynamicMatrix(const RowSizeType num_rows, const ColSizeType num_cols);
		DynamicMatrix(const DynamicMatrix& arg);
		DynamicMatrix(DynamicMatrix&& arg) noexcept;
		virtual ~DynamicMatrix();

		/// Returns the beginning of the connectivity with type of
		/// const pointer to const pointers of objects.  Only the objects are multable.
		T * const begin() const;
		/// Returns the end of the connectivity with type of const pointer to const pointers of const objects.
		/// Note: this points just beyond the array.  DO NOT use any methods.  Only use to test iterator.
		T * const end() const;
		/// Returns the number of rows of the container
		const RowSizeType& n_rows() const;
		/// Returns the number of columns of the container
		const ColSizeType& n_cols() const;

		/// Resizes the array to a certain number of objects.  The default constructor is called for each object.
		void resize(const RowSizeType num_rows, const ColSizeType num_cols);

		const T & operator () (const RowSizeType i, const ColSizeType j) const;
		T & operator () (const RowSizeType i, const ColSizeType j);
		/// Returns a very simple wrapper that provides the begin and end iterator for a row.
		/// Note: this would need to return a smarter iterator if get_col is to be implemented.
		const DynamicMatrixRow<T, ColSizeType> get_row(const RowSizeType i);
		/// Assign operator that takes an iterable lvalue container.
		/// Note: a copy is made for each object in objects.
		/// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
		/// error since the unique_ptr will be const and not movable.
		/// Convert the initializer list to an array to make the items moveable
		template<typename EntryPtrContainer>
		DynamicMatrix& set_row(const RowSizeType i, const EntryPtrContainer & objects);
		/// Assign operator that takes an iterable rvalue container.
		/// Note: each object is moved to this container from objects.
		/// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
		/// error since the unique_ptr will be const and not movable.
		/// Convert the initializer list to an array to make the items moveable
		template<typename EntryPtrContainer>
		DynamicMatrix& set_row(const RowSizeType i, EntryPtrContainer && objects);
		DynamicMatrix& operator = (const DynamicMatrix &other);

		/// Clears the container and frees the memory
		void clear();

	protected:
		// A pointer to array of objects that form the matrix.  The objects will
		// be stored first by row then by col.  For example,
		// { (r0, c0), ..., (r0, cN), ..., (rM, c0), ..., (rM, cN) }
		// where N is the col_size_ and M is the row_size_
		T* objects_;
		RowSizeType row_size_;
		ColSizeType col_size_;
	};

	/////////////////
	// Definitions //
	/////////////////

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline DynamicMatrix<T, RowSizeType, ColSizeType>::DynamicMatrix()
	{
		objects_ = nullptr;
		row_size_ = 0;
		col_size_ = 0;
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline DynamicMatrix<T, RowSizeType, ColSizeType>::DynamicMatrix(const RowSizeType num_rows, const ColSizeType num_cols)
		: objects_(new T[num_rows * num_cols]),
		row_size_(static_cast<RowSizeType>(num_rows)),
		col_size_(static_cast<ColSizeType>(num_cols))
	{
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline DynamicMatrix<T, RowSizeType, ColSizeType>::DynamicMatrix(const DynamicMatrix& arg)
		: objects_(new T[arg.row_size_ * arg.col_size_]),
		row_size_(arg.row_size_),
		col_size_(arg.col_size_)
	{
		assert(arg.row_size_ * arg.col_size_ > 0);
		std::copy(&arg.objects_[0], &arg.objects_[arg.size_], stdext::checked_array_iterator<T*>(objects_, row_size_ * col_size_));
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline DynamicMatrix<T, RowSizeType, ColSizeType>::DynamicMatrix(DynamicMatrix && arg) noexcept
	{
		std::swap(objects_, arg.objects_);
		std::swap(row_size_, arg.row_size_);
		std::swap(col_size_, arg.col_size_);
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline DynamicMatrix<T, RowSizeType, ColSizeType>::~DynamicMatrix()
	{
		clear();
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline T * const DynamicMatrix<T, RowSizeType, ColSizeType>::begin() const
	{
		return &objects_[0];
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline T * const DynamicMatrix<T, RowSizeType, ColSizeType>::end() const
	{
		return &objects_[row_size_ * col_size_];
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline const RowSizeType & DynamicMatrix<T, RowSizeType, ColSizeType>::n_rows() const
	{
		return row_size_;
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline const ColSizeType & DynamicMatrix<T, RowSizeType, ColSizeType>::n_cols() const
	{
		return col_size_;
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline void DynamicMatrix<T, RowSizeType, ColSizeType>::resize(const RowSizeType num_rows, const ColSizeType num_cols)
	{
        if (objects_ != nullptr)
        {
            // If the new size is larger, then do a conservative resize and move the memory over to the new one
            if ((num_rows * num_cols) > (row_size_ * col_size_))
            {
                T * tmp = objects_;
                objects_ = new T[num_rows * num_cols]();
                std::move(&tmp[0], &tmp[row_size_ * col_size_], stdext::checked_array_iterator<T*>(objects_, row_size_ * col_size_));
                delete[] tmp;
            }
            // If the new size is smaller, then throw away the old data
            else
            {
                delete[] objects_;
                objects_ = new T[num_rows * num_cols]();
            }
        }
        else
        {
            objects_ = new T[num_rows * num_cols]();
        }
		row_size_ = num_rows;
		col_size_ = num_cols;
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline const T & DynamicMatrix<T, RowSizeType, ColSizeType>::operator()(const RowSizeType i, const ColSizeType j) const
	{
		assert(i >= 0 && i < row_size_);
		assert(j >= 0 && j < col_size_);
		return objects_[col_size_ * i + j];
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline T& DynamicMatrix<T, RowSizeType, ColSizeType>::operator()(const RowSizeType i, const ColSizeType j)
	{
		assert(i >= 0 && i < row_size_);
		assert(j >= 0 && j < col_size_);
		return objects_[col_size_ * i + j];
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline const DynamicMatrixRow<T, ColSizeType> DynamicMatrix<T, RowSizeType, ColSizeType>::get_row(const RowSizeType i)
	{
		assert(i >= 0 && i < row_size_);
		return DynamicMatrixRow<T, ColSizeType>(&objects_[col_size_ * i], &objects_[col_size_ * i + col_size_]);
	}


	template<typename T, typename RowSizeType, typename ColSizeType>
	template<typename EntryPtrContainer>
	inline DynamicMatrix<T, RowSizeType, ColSizeType> & DynamicMatrix<T, RowSizeType, ColSizeType>::set_row(const RowSizeType i, const EntryPtrContainer & objects)
	{
		assert(i >= 0 && i < row_size_);
		assert(col_size_ == objects.size());
		auto obj_iter = objects.begin();
		for (auto j = 0; j < col_size_; j++, obj_iter++)
		{
			(*this)(i, j) = *obj_iter;
		}
	}

	template<typename T, typename RowSizeType, typename ColSizeType>
	template<typename EntryPtrContainer>
	inline DynamicMatrix<T, RowSizeType, ColSizeType> & DynamicMatrix<T, RowSizeType, ColSizeType>::set_row(const RowSizeType i, EntryPtrContainer && objects)
	{
		assert(i >= 0 && i < row_size_);
		assert(col_size_ == objects.size());
		auto obj_iter = objects.begin();
		for (auto j = 0; j < col_size_; j++, obj_iter++)
		{
			(*this)(i, j) = std::move(*obj_iter);
		}
		return *this;
	}

        /*
	template<typename T, typename RowSizeType, typename ColSizeType>
	inline DynamicMatrix<T, RowSizeType, ColSizeType> & DynamicMatrix<T, RowSizeType, ColSizeType>::operator=(const DynamicMatrix & arg)
	{
		assert(i >= 0 && i < row_size_);
		assert(col_size_ == arg.objects.size());
		auto obj_iter = arg.objects.begin();
		for (auto j = 0; j < col_size_; j++, obj_iter++)
		{
			(*this)(i, j) = std::move(*obj_iter);
		}
		return *this;
	}
        */

	template<typename T, typename RowSizeType, typename ColSizeType>
	inline void DynamicMatrix<T, RowSizeType, ColSizeType>::clear()
	{
		delete[] objects_;
		row_size_ = 0;
		col_size_ = 0;
	}

}
