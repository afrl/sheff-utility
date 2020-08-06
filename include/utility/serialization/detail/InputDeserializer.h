#pragma once

#include "Serializer.h"
#include "utility/Slice.h"

#include <istream>

namespace utility
{
/// @brief A class that reads and deserializes data from an std::istream.
///
/// This class follows the pattern of serialization pattern this project uses.
/// Refer to serialization.md for a summary of the overall strategy.
class InputDeserializer : public Serializer<InputDeserializer>
{
  public:
    InputDeserializer(std::istream& in)
      : in_(in)
    {
    }

  private:
    std::istream& in_;

    /// Grant access to parent class so it can call process(...)
    friend class Serializer<InputDeserializer>;

    /// Deserializes fundamental types.
    /// @param name - (ignored for this serializer type)
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(T& v, const std::string& name = "")
    {
        in_.read(reinterpret_cast<char*>(&v), sizeof(T));
    }

    /// Deserializes a slice.
    /// @param name - (ignored for this serializer type)
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(Slice<T> slice, const std::string& name = "")
    {
        in_.read(reinterpret_cast<char* const>(slice.data()), sizeof(T) * slice.size());
    }

    /// The fall back for non-fundamental types.  This passes the argument on to the
    /// Serializer::forward_to_deserialize method, which will find the appropriate deserialize
    /// method/function and call process() on the fundamental types within any structures.
    /// @param name - (ignored for this serializer type)
    template<typename T,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(T& v, const std::string& name = "")
    {
        Serializer<InputDeserializer>::forward_to_deserialize(v);
    }
};
}