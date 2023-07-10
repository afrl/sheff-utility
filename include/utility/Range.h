#pragma once

/*
  Copyright 2012-2016 Konrad Rudolph

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <iterator>
#include <type_traits>

namespace utility
{
/// A range to be used for array's.
/// \code{cpp}
/// std::array<double, 3> values = { 0.5, 1.0, 2.0 };
/// for (double& value : range_for_array<double>(&values[0], &values[0] + 3))
/// {
///     value = 0.0;
/// }
/// // The array is now 0
/// \endcode
/// Added by Keith Ballard
template<typename T>
struct range_for_array
{
    range_for_array(T* begin_ptr, T* end_ptr)
      : begin_ptr_(begin_ptr)
      , end_ptr_(end_ptr)
    {
    }

    T* begin() const
    {
        return begin_ptr_;
    }
    T* end() const
    {
        return end_ptr_;
    }

    /// A template to convert the range to another derived type of T.  A compiler
    /// error will be throw if Derived is not actually a derived type of T.
    template<typename Derived,
             typename std::enable_if<std::is_base_of<T, Derived>::value, T>::type* = nullptr>
    range_for_array<Derived> as() const
    {
        return { static_cast<Derived*>(begin_ptr_), static_cast<Derived*>(end_ptr_) };
    }

  private:
    T* const begin_ptr_;
    T* const end_ptr_;
};

namespace detail
{
template<typename T>
struct range_iter_base
{
    using iterator_category = std::input_iterator_tag;
    using value_type        = T;
    using difference_type   = T;
    using pointer           = T*;
    using reference         = T&;

    range_iter_base(T current)
      : current(current)
    {
    }

    T operator*() const
    {
        return current;
    }

    T const* operator->() const
    {
        return &current;
    }

    range_iter_base& operator++()
    {
        ++current;
        return *this;
    }

    range_iter_base operator++(int)
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    bool operator==(range_iter_base const& other) const
    {
        return current == other.current;
    }

    bool operator!=(range_iter_base const& other) const
    {
        return !(*this == other);
    }

  protected:
    T current;
};

} // namespace detail

template<typename T>
struct range_proxy
{
    struct iterator : detail::range_iter_base<T>
    {
        iterator(T current)
          : detail::range_iter_base<T>(current)
        {
        }
    };

    struct step_range_proxy
    {
        struct iter : detail::range_iter_base<T>
        {
            iter(T current, T step)
              : detail::range_iter_base<T>(current)
              , step(step)
            {
            }

            using detail::range_iter_base<T>::current;

            iter& operator++()
            {
                current += step;
                return *this;
            }

            iter operator++(int)
            {
                auto copy = *this;
                ++*this;
                return copy;
            }

            // Loses commutativity. Iterator-based ranges are simply broken. :-(
            bool operator==(iter const& other) const
            {
                return step > 0 ? current >= other.current : current < other.current;
            }

            bool operator!=(iter const& other) const
            {
                return not(*this == other);
            }

          private:
            T step;
        };

        step_range_proxy(T begin, T end, T step)
          : begin_(begin, step)
          , end_(end, step)
        {
        }

        iter begin() const
        {
            return begin_;
        }

        iter end() const
        {
            return end_;
        }

      private:
        iter begin_;
        iter end_;
    };

    range_proxy(T begin, T end)
      : begin_(begin)
      , end_(end)
    {
    }

    step_range_proxy step(T step)
    {
        return { *begin_, *end_, step };
    }

    iterator begin() const
    {
        return begin_;
    }

    iterator end() const
    {
        return end_;
    }

  private:
    iterator begin_;
    iterator end_;
};

template<typename T>
struct infinite_range_proxy
{
    struct iterator : detail::range_iter_base<T>
    {
        iterator(T current = T())
          : detail::range_iter_base<T>(current)
        {
        }

        bool operator==(iterator const&) const
        {
            return false;
        }

        bool operator!=(iterator const&) const
        {
            return true;
        }
    };

    struct step_range_proxy
    {
        struct iter : detail::range_iter_base<T>
        {
            iter(T current = T(), T step = T())
              : detail::range_iter_base<T>(current)
              , step(step)
            {
            }

            using detail::range_iter_base<T>::current;

            iter& operator++()
            {
                current += step;
                return *this;
            }

            iter operator++(int)
            {
                auto copy = *this;
                ++*this;
                return copy;
            }

            bool operator==(iter const&) const
            {
                return false;
            }

            bool operator!=(iter const&) const
            {
                return true;
            }

          private:
            T step;
        };

        step_range_proxy(T begin, T step)
          : begin_(begin, step)
        {
        }

        iter begin() const
        {
            return begin_;
        }

        iter end() const
        {
            return iter();
        }

      private:
        iter begin_;
    };

    infinite_range_proxy(T begin)
      : begin_(begin)
    {
    }

    step_range_proxy step(T step)
    {
        return step_range_proxy(*begin_, step);
    }

    iterator begin() const
    {
        return begin_;
    }

    iterator end() const
    {
        return iterator();
    }

  private:
    iterator begin_;
};

template<typename T>
range_proxy<T> range(T begin, T end)
{
    return { begin, end };
}

/// Note: this has been modified from Konrad Rudolph's implementation to follow a Pythonic
/// convention. The range used to be a infinite range given a beginning, but this is not
/// practically useful often.  More often, we want a 0-based range to a specified end.
template<typename T>
range_proxy<T> range(T end)
{
    return { T(0), end };
}

/// Similar to the signature above, but allows static casting to another type, since this
/// is a consistent need in the framework.
template<typename T, typename V>
range_proxy<T> range_as(V end)
{
    return { T(0), static_cast<T>(end) };
}

template<typename T, typename V, typename W>
range_proxy<T> range_as(V begin, W end)
{
    return { static_cast<T>(begin), static_cast<T>(end) };
}

template<typename T, typename V, typename W>
range_proxy<T>::step_range_proxy reversed_range_as(V begin, W end)
{
    return { static_cast<T>(end - 1), static_cast<T>(begin - 1), -1 };
}

namespace traits
{

template<typename C>
struct has_size
{
    template<typename T>
    static auto check(T*) ->
      typename std::is_integral<decltype(std::declval<T const>().size())>::type;

    template<typename>
    static auto check(...) -> std::false_type;

    using type                  = decltype(check<C>(0));
    static constexpr bool value = type::value;
};

} // namespace traits

template<typename C, typename = typename std::enable_if<traits::has_size<C>::value>>
auto indices(C const& cont) -> range_proxy<decltype(cont.size())>
{
    return { 0, cont.size() };
}

/// Similar to the signature above, but allows static casting to another type, since this
/// is a consistent needed in the framework.
template<typename T, typename C, typename = typename std::enable_if<traits::has_size<C>::value>>
auto indices_as(C const& cont) -> range_proxy<T>
{
    return { T(0), static_cast<T>(cont.size()) };
}

template<typename T, std::size_t N>
range_proxy<std::size_t> indices(T (&)[N])
{
    return { 0, N };
}

template<typename T>
range_proxy<typename std::initializer_list<T>::size_type> indices(std::initializer_list<T>&& cont)
{
    return { 0, cont.size() };
}

/// @brief A helper struct for closed_range
/// @tparam T - Index type
template<typename T>
struct IndexPair
{
    T i;
    T j;


    IndexPair(T i_a, T j_a, T N_a)
    {
        i = i_a;
        j = j_a;
        N = N_a;
    }

    IndexPair& operator++()
    {
        ++i;
        ++j;
        // Note: i will go to N, which we will leverage to close the iterator
        if (j == N)
        {
            j = 0;
        }
        return *this;
    }

    IndexPair operator++(int)
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    bool operator==(IndexPair const& other) const
    {
        return i == other.i && j == other.j;
    }

    bool operator!=(IndexPair const& other) const
    {
        return !(*this == other);
    }

  private:
    T N;
};

/// Provides a range over pairs of { i, i + 1 }, including { N-1, 0 } to close the loop
template<typename T>
range_proxy<IndexPair<T>> closed_range(T N)
{
    // This is a quirk we are exploiting.  i goes to N and does not loop.  If we didn't use this
    // then we would need to differientate between the first {0, 1} and the last {0, 1} with a
    // counter or something.
    return { { 0, 1, N }, { N, 1, N } };
}

/// Provides a range over pairs of { i, i + 1 }, ending at { N-2, N-1 }
template<typename T>
range_proxy<IndexPair<T>> open_range(T N)
{
    return { { 0, 1, N }, { N-1, 0, N } };
}

} // namespace utility
