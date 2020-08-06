#pragma once

#include "Serializer.h"

namespace utility
{
/// @brief A class that counts the bytes that a serialization of the data would have.
///
/// This class follows the pattern of serialization pattern this project uses.
/// Refer to serialization.md for a summary of the overall strategy.
class SerialByteCounter : public Serializer<SerialByteCounter>
{
  public:
    SerialByteCounter()
      : n_bytes_(0)
    {
    }

    /// Returns the number of bytes of all objects passed to the operator() thus far.
    std::size_t n_bytes() const
    {
        return n_bytes_;
    }

  private:
    std::size_t n_bytes_;

    /// Grant access to parent class so it can call process(...)
    friend class Serializer<SerialByteCounter>;

    /// Counts the number of bytes for fundamental types.
    /// @param name - (ignored for this serializer type)
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const T& v, const std::string& name = "")
    {
        n_bytes_ += sizeof(T);
    }

    /// Counts the number of bytes for a slice.
    /// @param name - (ignored for this serializer type)
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const Slice<T>& slice, const std::string& name = "")
    {
        n_bytes_ += sizeof(T) * slice.size();
    }

    /// The fall back for non-fundamental types.  This passes the argument on to the
    /// Serializer::forward_to_serialize method, which will find the appropriate serialize
    /// method/function and call process() on the fundamental types within any structures.
    /// @param name - (ignored for this serializer type)
    template<typename T,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const T& v, const std::string& name = "")
    {
        Serializer<SerialByteCounter>::forward_to_serialize(v);
    }
};
}
