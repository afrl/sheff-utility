#pragma once

#include "CrossplatformHelpers.h"

#include <assert.h>
#include <limits>
#include <memory>
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
    using size_type  = SizeType;

    DynamicArray();
    DynamicArray(const SizeType num_objects);
    /// A constructor that takes an iteratable container class that contains
    /// T objects.  It must have begin and end methods.
    /// Note: this will try to move the data from objects to this, though the type
    /// must support a move assignment operator (otherwise a copy will be made).
    template<typename TContainer>
    DynamicArray(TContainer&& objects);
    /// /see DynamicArray(TContainer&& objects)
    template<typename TContainer>
    DynamicArray(const TContainer& objects);
    /// A constructor that takes an initializer list, use like:
    /// DynamicArray<int> int_array({ 1, 2 });
    /// DynamicArray<int*> int_p_array({ &int_1, &int_2 });
    /// Note: since the objects in an initializer list are const, this must copy them
    /// even though the initializer_list is an rvalue.
    /// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
    /// error since the unique_ptr will be const and not movable.
    /// Convert the initializer list to an array to use the constructor above.
    DynamicArray(std::initializer_list<T> objects);
    DynamicArray(const DynamicArray& arg);
    DynamicArray(DynamicArray&& arg) noexcept;
    virtual ~DynamicArray();

    /// Returns the beginning of the connectivity with type of
    /// const pointer to const pointers of objects.  Only the objects are multable.
    T* const begin() const;
    /// Returns the end of the connectivity with type of const pointer to const pointers of const
    /// objects. Note: this points just beyond the array.  DO NOT use any methods.  Only use to test
    /// iterator.
    T* const end() const;
    /// Returns the first node in the connectivity list
    T& front() const;
    /// Returns the last node in the connectivity list
    T& back() const;
    /// Returns the size of the container
    const SizeType& size() const;

    /// Resizes the array to a certain number of objects.  The default constructor is called for
    /// each object.
    void resize(const SizeType num_objects);
    /// Appends an element to the end, reallocates for each addition
    void push_back(const T& entry);
    void push_back(T&& entry);
    /// Appends a duplicate of the indicated object to the end of the array.
    void push_back_duplicate(const SizeType index_to_duplicate);
    /// Appends an empty object to the back of the array.  This is useful if the array
    /// is storing another array of unique_ptr objects, causing push_back to cause a problem
    /// with the copy.
    void push_back_empty();
    /// Inserts the given entry just before the indicated position. (shifts the elements at
    /// position and higher)
    void insert(const SizeType position, const T& entry);
    /// Inserts the given entry just before the indicated position. (shifts the elements at
    /// position and higher)
    void insert(const SizeType position, T&& entry);

    const T& operator[](const SizeType i) const;
    T& operator[](const SizeType i);
    /// Assign operator that takes an iterable lvalue container.
    /// Note: a copy is made for each object in objects.
    /// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
    /// error since the unique_ptr will be const and not movable.
    /// Convert the initializer list to an array to make the items moveable
    template<typename EntryPtrContainer>
    DynamicArray& operator=(const EntryPtrContainer& objects);
    /// Assign operator that takes an iterable rvalue container.
    /// Note: each object is moved to this container from objects.
    /// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
    /// error since the unique_ptr will be const and not movable.
    /// Convert the initializer list to an array to make the items moveable
    template<typename EntryPtrContainer>
    DynamicArray& operator=(EntryPtrContainer&& objects);
    DynamicArray& operator=(const DynamicArray& other);

    /// Returns true if the size of the two containers are the same and operator== returns true for
    /// each pair of objects in the two containers.
    bool operator==(const DynamicArray& other) const;

    /// Get a copy of the connectivity as a vector
    /// Note: if T = std::unique_ptr<...>, this method will cause a deleted function
    /// error since the unique_ptr is not copyable.
    const std::vector<T> copy_as_vector() const;

    void swap(DynamicArray& other);

    /// Clears the container and frees the memory
    void clear();

  protected:
    std::allocator<T> allocator_;
    T* objects_;
    SizeType size_;

    /// Destroys objects and deallocates memory, returning objects to a nullptr
    /// and size to 0.
    void cleanup_if_needed(T*& objects, SizeType& size);

    void resize_array(const SizeType& new_size);

    /// @brief Returns true if the pointer exists in the array.
    bool address_is_in_array(const T* ptr) const;
};

/////////////////
// Definitions //
/////////////////

template<typename T, typename SizeType>
inline DynamicArray<T, SizeType>::DynamicArray()
{
    objects_ = nullptr;
    size_    = 0;
}

template<typename T, typename SizeType>
inline DynamicArray<T, SizeType>::DynamicArray(const SizeType num_objects)
{
    if (num_objects == 0)
    {
        objects_ = nullptr;
        size_    = 0;
    }
    else
    {
        resize_array(num_objects);

        SizeType pos = 0;
        for (auto& obj : objects)
        {
            std::allocator_traits<std::allocator<T>>::construct(allocator_, objects_ + pos++);
        }
    }
}

template<typename T, typename SizeType>
template<typename TContainer>
inline DynamicArray<T, SizeType>::DynamicArray(TContainer&& objects)
  : DynamicArray()
{
    if (objects.size() > 0)
    {
        assert(objects.size() < std::numeric_limits<SizeType>::max());
        resize_array(static_cast<SizeType>(objects.size()));

        SizeType pos = 0;
        for (auto& obj : objects)
        {
            std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                                objects_ + pos++,
                                                                std::move(obj));
        }
    }
}

template<typename T, typename SizeType>
template<typename TContainer>
inline DynamicArray<T, SizeType>::DynamicArray(const TContainer& objects)
  : DynamicArray()
{
    if (objects.size() > 0)
    {
        assert(objects.size() < std::numeric_limits<SizeType>::max());
        resize_array(static_cast<SizeType>(objects.size()));

        SizeType pos = 0;
        for (auto& obj : objects)
        {
            std::allocator_traits<std::allocator<T>>::construct(allocator_, objects_ + pos++, obj);
        }
    }
}

template<typename T, typename SizeType>
inline DynamicArray<T, SizeType>::DynamicArray(std::initializer_list<T> objects)
  : DynamicArray()
{
    if (objects.size() > 0)
    {
        objects_   = allocator_.allocate(objects.size());
        size_      = static_cast<SizeType>(objects.size());
        SizeType i = 0;
        for (auto obj : objects)
        {
            std::allocator_traits<std::allocator<T>>::construct(allocator_, objects_ + i++, obj);
        }
    }
}

template<typename T, typename SizeType>
inline DynamicArray<T, SizeType>::DynamicArray(const DynamicArray& arg)
  : DynamicArray()
{
    if (arg.size_ > 0)
    {
        objects_ = allocator_.allocate(arg.size_);
        size_    = static_cast<SizeType>(arg.size_);
        for (SizeType i = 0; i < size_; i++)
        {
            std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                                objects_ + i++,
                                                                arg.objects_[i]);
        }
    }
}

template<typename T, typename SizeType>
inline DynamicArray<T, SizeType>::DynamicArray(DynamicArray&& arg) noexcept
  : DynamicArray()
{
    objects_     = arg.objects_;
    size_        = arg.size_;
    arg.objects_ = nullptr;
    arg.size_    = 0;
}

template<typename T, typename SizeType>
inline DynamicArray<T, SizeType>::~DynamicArray()
{
    clear();
}

template<typename T, typename SizeType>
inline T* const DynamicArray<T, SizeType>::begin() const
{
    return &objects_[0];
}

template<typename T, typename SizeType>
inline T* const DynamicArray<T, SizeType>::end() const
{
    return &objects_[size_];
}

template<typename T, typename SizeType>
inline T& DynamicArray<T, SizeType>::front() const
{
    return objects_[0];
}

template<typename T, typename SizeType>
inline T& DynamicArray<T, SizeType>::back() const
{
    return objects_[size_ - 1];
}

template<typename T, typename SizeType>
inline const SizeType& DynamicArray<T, SizeType>::size() const
{
    return size_;
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::push_back(const T& entry)
{
    // Since, resize will change the address of the objects, you can't push back a copy in the
    // array.  Use push_back_duplicate instead.
    assert(!address_is_in_array(&entry));

    resize_array(size_ + 1);
    std::allocator_traits<std::allocator<T>>::construct(allocator_, &objects_[size_ - 1], entry);
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::push_back(T&& entry)
{
    resize_array(size_ + 1);
    std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                        &objects_[size_ - 1],
                                                        std::move(entry));
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::push_back_duplicate(const SizeType index_to_duplicate)
{
    resize_array(size_ + 1);
    std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                        &objects_[size_ - 1],
                                                        objects_[index_to_duplicate]);
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::resize(const SizeType num_objects)
{
    auto tmp_size = size_;
    resize_array(num_objects);

    for (auto i = tmp_size; i < size_; i++)
    {
        std::allocator_traits<std::allocator<T>>::construct(allocator_, objects_ + i);
    }
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::push_back_empty()
{
    resize_array(size_ + 1);
    std::allocator_traits<std::allocator<T>>::construct(allocator_, objects_ + size_ - 1);
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::insert(const SizeType position, const T& entry)
{
    resize_array(size_ + 1);
    // Note: second condition in for statement is for unsigned types
    for (SizeType i = size_ - 1; i >= position && i < size_; i--)
    {
        std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                            &objects_[i],
                                                            std::move(objects_[i - 1]));
    }
    std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                        &objects_[position],
                                                        entry);
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::insert(const SizeType position, T&& entry)
{
    resize_array(size_ + 1);
    for (SizeType i = size_ - 1; i >= position; i--)
    {
        std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                            &objects_[i],
                                                            std::move(objects_[i-1]));
    }
    std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                        &objects_[position],
                                                        std::move(entry));
}

template<typename T, typename SizeType>
inline const T& DynamicArray<T, SizeType>::operator[](const SizeType i) const
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
inline DynamicArray<T, SizeType>& DynamicArray<T, SizeType>::operator=(
  const EntryPtrContainer& objects)
{
    assert(objects.size() < std::numeric_limits<SizeType>::max());
    resize_array(objects.size());

    // Note: copy requires =operator, so we will use copy constructor instead
    SizeType i = 0;
    for (auto obj : objects)
    {
        std::allocator_traits<std::allocator<T>>::construct(allocator_, objects_ + i++, obj);
    }
    return *this;
}

template<typename T, typename SizeType>
template<typename EntryPtrContainer>
inline DynamicArray<T, SizeType>& DynamicArray<T, SizeType>::operator=(EntryPtrContainer&& objects)
{
    assert(objects.size() < std::numeric_limits<SizeType>::max());
    resize_array(static_cast<SizeType>(objects.size()));

    SizeType pos = 0;
    for (T& obj : objects)
    {
        std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                            objects_ + pos++,
                                                            std::move(obj));
    }
    return *this;
}

template<typename T, typename SizeType>
inline DynamicArray<T, SizeType>& DynamicArray<T, SizeType>::operator=(const DynamicArray& other)
{
    resize_array(other.size());

    // Note: copy requires =operator, so we will use copy constructor instead
    SizeType i = 0;
    for (auto obj : other)
    {
        std::allocator_traits<std::allocator<T>>::construct(allocator_, objects_ + i++, obj);
    }
    return *this;
}

template<typename T, typename SizeType>
inline bool DynamicArray<T, SizeType>::operator==(const DynamicArray& other) const
{
    if (size_ != other.size_)
    {
        return false;
    }
    bool comparison = true;
    for (auto i : utility::range(size_))
    {
        comparison &= objects_[i] == other.objects_[i];
    }
    return comparison;
}

template<typename T, typename SizeType>
inline const std::vector<T> DynamicArray<T, SizeType>::copy_as_vector() const
{
    std::vector<T> conn_copy(size());
    std::copy(&objects_[0], &objects_[size_], conn_copy.begin());
    return conn_copy;
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::swap(DynamicArray& other)
{
    std::swap(this->objects_, other.objects_);
    std::swap(this->size_, other.size_);
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::clear()
{
    cleanup_if_needed(objects_, size_);
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::cleanup_if_needed(T*& objects, SizeType& size)
{
    if (objects != nullptr)
    {
        for (auto i = 0; i < size; i++)
        {
            allocator_.destroy(objects + i);
        }
        allocator_.deallocate(objects, size);
    }

    objects = nullptr;
    size    = 0;
}

template<typename T, typename SizeType>
inline void DynamicArray<T, SizeType>::resize_array(const SizeType& new_size)
{
    if (new_size == size_)
    {
        return;
    }

    if (objects_)
    {
        auto tmp      = objects_;
        auto tmp_size = size_;

        size_         = new_size;
        objects_      = allocator_.allocate(size_);
        auto loop_end = new_size < tmp_size ? new_size : tmp_size;
        for (SizeType i = 0; i < loop_end; i++)
        {
            std::allocator_traits<std::allocator<T>>::construct(allocator_,
                                                                &objects_[i],
                                                                std::move(tmp[i]));
        }

        cleanup_if_needed(tmp, tmp_size);
    }
    else
    {
        size_    = new_size;
        objects_ = allocator_.allocate(size_);
    }
}

template<typename T, typename SizeType>
inline bool DynamicArray<T, SizeType>::address_is_in_array(const T* ptr) const
{
    return ptr >= objects_ && ptr <= objects_ + size_;
}

}
