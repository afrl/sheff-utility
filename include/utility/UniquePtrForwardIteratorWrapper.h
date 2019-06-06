#pragma once

#include <iterator>

namespace utility
{
    /// Wraps an iterator for a container of unique_ptr objects that you want an
    /// interface to the underlying data (not the unique_ptr).  This effectively
    /// provides an iterface that allows you to treat an iterator as if the container
    /// actually contained the objects instead of unique_ptr's of objects.
    /// For example:
    /// \code{.cpp}
    /// class Foo {
    ///     bool a() const { return true; }
    /// };
    /// using Container = std::vector<std::unique_ptr<Foo>>;
    /// using iterator = UniquePtrForwardIteratorWrapper<Container::iterator>;
    /// using const_iterator = UniquePtrForwardIteratorWrapper<Container::const_iterator>;
    ///
    /// Container some_container;
    /// // Fill the container...
    /// auto begin_iter = iterator(some_container.begin());
    /// (*begin_iter).a(); // is valid
    /// \endcode
    template<typename UnderlyingIterator>
    class UniquePtrForwardIteratorWrapper
    {
    public:
        using value_type = typename UnderlyingIterator::value_type::element_type;
        using pointer = value_type *;
        using reference = value_type &;
        using iterator_category = std::forward_iterator_tag;
        using underlying_iterator = UnderlyingIterator;

        UniquePtrForwardIteratorWrapper() = delete;
        UniquePtrForwardIteratorWrapper(const underlying_iterator& iterator)
            : underlying_iter_(iterator) {}
        ~UniquePtrForwardIteratorWrapper() = default;
        UniquePtrForwardIteratorWrapper& operator=(const UniquePtrForwardIteratorWrapper& other) = default;

        /// Dereference operator
        reference operator*() const
        {
            return **underlying_iter_;
        }
        /// Pointer operator
        pointer operator->() const
        {
            return underlying_iter_->get();
        }
        /// Prefix increment
        UniquePtrForwardIteratorWrapper& operator++()
        {
            underlying_iter_++;
            return *this;
        }
        /// Postfix increment
        UniquePtrForwardIteratorWrapper operator++(int)
        {
            auto iter = *this;
            underlying_iter_++;
            return iter;
        }

        friend void swap(
            UniquePtrForwardIteratorWrapper&,
            UniquePtrForwardIteratorWrapper&);
        friend bool operator==(
            const UniquePtrForwardIteratorWrapper&,
            const UniquePtrForwardIteratorWrapper&);
        
        bool operator!=(
            const UniquePtrForwardIteratorWrapper& rhs)
        {
            return !(underlying_iter_ == rhs.underlying_iter_);
        }

    private:
        underlying_iterator underlying_iter_;
    };

    template<typename UnderlyingIterator>
    void swap(
        UniquePtrForwardIteratorWrapper<UnderlyingIterator>& lhs,
        UniquePtrForwardIteratorWrapper<UnderlyingIterator>& rhs)
    {
        std::swap(lhs.underlying_iter_, rhs.underlying_iter_);
    }

    template<typename UnderlyingIterator>
    bool operator==(
        const UniquePtrForwardIteratorWrapper<UnderlyingIterator>& lhs,
        const UniquePtrForwardIteratorWrapper<UnderlyingIterator>& rhs)
    {
        return lhs.underlying_iter_ == rhs.underlying_iter_;
    }

    /*
    template<typename UnderlyingIterator>
    bool operator!=(
        const UniquePtrForwardIteratorWrapper<UnderlyingIterator>& lhs,
        const UniquePtrForwardIteratorWrapper<UnderlyingIterator>& rhs)
    {
        return !(underlying_iter_ == other.underlying_iter_);
    }
    */
}