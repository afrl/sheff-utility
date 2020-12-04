#pragma once

#include "Serializer.h"
#include "utility/Slice.h"
#include "utility/SliceList.h"

#include <ostream>

namespace utility
{

/// @brief A class that writes a serialization to an std::ostream.
///
/// This class follows the pattern of serialization pattern this project uses.
/// Refer to serialization.md for a summary of the overall strategy.
class OutputSerializer : public Serializer<OutputSerializer>
{
  public:
    OutputSerializer(std::ostream& out)
      : out_(out)
    {
    }

  private:
    std::ostream& out_;

    /// Grant access to parent class so it can call process(...)
    friend class Serializer<OutputSerializer>;

    /// Serializes fundamental types.
    /// @param name - (ignored for this serializer type)
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const T& v, const std::string& name = "")
    {
        out_.write(reinterpret_cast<const char*>(&v), sizeof(T));
    }

    /// Serializes a slice.
    /// @param name - (ignored for this serializer type)
    template<typename T,
             typename SizeType,
             typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const Slice<T, SizeType>& slice, const std::string& name = "")
    {
        out_.write(reinterpret_cast<const char*>(slice.data()), sizeof(T) * slice.size());
    }

    /// The fall back for non-fundamental types.  This passes the argument on to the
    /// Serializer::forward_to_serialize method, which will find the appropriate serialize
    /// method/function and call process() on the fundamental types within any structures.
    /// @param name - (ignored for this serializer type)
    template<typename T,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const T& v, const std::string& name = "")
    {
        Serializer<OutputSerializer>::forward_to_serialize(v);
    }
};
}
