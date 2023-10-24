#pragma once

#include <bitset>

namespace utility
{
    /// A helper function to mask a enum class to bits.
    template<typename T>
    int mask(const T& flag)
    {
        return 1 << static_cast<typename std::underlying_type<T>::type>(flag);
    }

    /// An generalization of std::bitset for enum class types.  Requires enum class
    /// to have and entry called `_end` at the end of the enum
    /// For example:
    /// \code{cpp}
    /// enum class FlagsTest : int8_t
    /// {
    ///     A,
    ///     B,
    ///     _end
    /// };
    /// 
    /// bitset<FlagsTest> a = FlagsTest::A;
    /// bitset<FlagsTest> b = mask(FlagsTest::A) | mask(FlagsTest::B);
    /// \endcode
    template<typename T>
    class bitset : std::bitset<static_cast<unsigned long long>(T::_end)>
    {
    public:
        using Parent = std::bitset<static_cast<unsigned long long>(T::_end)>;

        // Constructors
        bitset() : Parent() {}
        bitset(const T& flag)
            : Parent()
        {
            this->flip(flag);
        }
        using Parent::Parent;

        // Comparison
        using Parent::operator==;
        bool operator!=(const bitset& other)
        {
            return !(*this == other);
        }

        // Element access
        inline bool contains(const T& flag) const
        {
            auto flag_index = static_cast<unsigned long long>(flag);
            return this->test(flag_index);
        }
        using Parent::all;
        using Parent::any;
        using Parent::none;
        using Parent::count;

        // Capacity
        using Parent::size;

        // Modifiers
        inline bitset& flip(const T& flag)
        {
            auto flag_index = static_cast<unsigned long long>(flag);
            Parent::flip(flag_index);
            return *this;
        }
        inline bitset& operator=(const T& flag)
        {
            this->reset();
            this->flip(flag);
            return *this;
        }
        using Parent::operator=;
        using Parent::operator&=;
        using Parent::operator|=;
        using Parent::operator^=;
        using Parent::operator~;
        using Parent::operator<<=;
        using Parent::operator>>=;
        using Parent::operator<<;
        using Parent::operator>>;

        // Conversions
        using Parent::to_string;
        using Parent::to_ulong;
        using Parent::to_ullong;
    };
}
