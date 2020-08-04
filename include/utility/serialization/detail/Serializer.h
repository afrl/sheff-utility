#pragma once

#include "utility/serialization/detail/NamedValue.h"
#include "utility/Slice.h"

#include <array>
#include <string>
#include <type_traits>
#include <vector>

namespace utility
{
template<typename T>
struct has_serialize;

template<typename T>
struct has_deserialize;

/// @brief Provides an empty base class for all other serializer objects to inherit from, allowing
///        templated methods to only be enabled for types deriving from Serializer.
///
/// Refer to serialization.md for a summary of the overall strategy.
class SerializerBase
{
  public:
    SerializerBase() {}
};

/// @brief Provides an empty base class for all other serializer objects to inherit from, allowing
///        templated methods to only be enabled for types deriving from Serializer.
///
/// Refer to serialization.md for a summary of the overall strategy.
template<typename DerivedSerializer>
class Serializer : SerializerBase
{
  public:
    Serializer() {}

    /// Provides primary public interface for DerivedSerializer
    template<typename... Args>
    inline void operator()(Args&&... args)
    {
        this->forward_to_derived_class(std::forward<Args>(args)...);
    }

  protected:
    /// Forwards the next element in the variadic list of arguments to the derived
    /// serializer's process method.
    template<typename T, typename... Args>
    inline void forward_to_derived_class(T&& v, Args&&... args)
    {
        static_cast<DerivedSerializer*>(this)->process(v);
        this->forward_to_derived_class(std::forward<Args>(args)...);
    }

    /// Forwards the next NamedValue in the variadic list of arguments to the derived
    /// serializer's process method.
    template<typename T, typename... Args>
    inline void forward_to_derived_class(NamedValue<T>&& v, Args&&... args)
    {
        static_cast<DerivedSerializer*>(this)->process(v.value, v.name);
        this->forward_to_derived_class(std::forward<Args>(args)...);
    }

    /// Forwards the last element in the variadic list of arguments to the derived
    /// serializer's process method.
    template<typename T>
    inline void forward_to_derived_class(T&& v)
    {
        static_cast<DerivedSerializer*>(this)->process(v);
    }

    /// Forwards the last NamedValue in the variadic list of arguments to the derived
    /// serializer's process method.
    template<typename T>
    inline void forward_to_derived_class(NamedValue<T>&& v)
    {
        static_cast<DerivedSerializer*>(this)->process(v.value, v.name);
    }


    /// If the derived serializer was not able to handle the type in its process method,
    /// it will call this method for non-fundamental types that have a .serialize method.
    template<typename T,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr,
             typename std::enable_if<has_serialize<T>::value, T>::type*        = nullptr>
    inline void forward_to_serialize(const T& v)
    {
        v.serialize(*this);
    }

    /// If the derived serializer was not able to handle the type in its process method,
    /// it will call this method for non-fundamental types for which there is free-function
    /// with the signature (where T is the non-fundamental type to serialize):
    /// \code{cpp}
    /// template<typename SerializerType,
    ///          typename std::enable_if<std::is_base_of<Serializer, SerializerType>::value,
    ///          T>::type* = nullptr>
    /// void serialize(SerializerType&, const T&);
    /// \endcode
    template<typename T,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr,
             typename std::enable_if<!has_serialize<T>::value, T>::type*       = nullptr>
    inline void forward_to_serialize(const T& v)
    {
        serialize(*this, v);
    }


    /// If the derived deserializer was not able to handle the type in its process method,
    /// it will call this method for non-fundamental types that have a .deserialize method.
    template<typename T,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr,
             typename std::enable_if<has_deserialize<T>::value, T>::type*      = nullptr>
    inline void forward_to_deserialize(T& v)
    {
        v.deserialize(*this);
    }

    /// If the derived deserializer was not able to handle the type in its process method,
    /// it will call this method for non-fundamental types for which there is free-function
    /// with the signature (where T is the non-fundamental type to deserialize):
    /// \code{cpp}
    /// template<typename SerializerType,
    ///          typename std::enable_if<std::is_base_of<Serializer, SerializerType>::value,
    ///          T>::type* = nullptr>
    /// void deserialize(SerializerType&, const T&);
    /// \endcode
    template<typename T,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr,
             typename std::enable_if<!has_deserialize<T>::value, T>::type*     = nullptr>
    inline void forward_to_deserialize(T& v)
    {
        deserialize(*this, v);
    }
};

/// @brief A Helper to test if a class has a .serialize(...) method that can take a
/// utility::Serializer object.
///
/// Usage: utility::has_serialize<T>::value will return 1 if the method exists, 0 otherwise.
template<typename T>
struct has_serialize
{
    // One of these two test method will exist, depending on if
    // std::declval<T>().serialize(std::declval<utility::Serializer&>()) is valid.  If T contains a
    // serialize method that can take a utility::Serializer object, then it succeeds and the return
    // type becomes std::true_type, otherwise, the return type is std::false_type as fall back.
    // Relies on SPINAE in case you are not familiar.
    template<typename C>
    static constexpr auto test(...) -> std::false_type;
    template<typename C>
    static constexpr auto test(int)
      -> decltype(std::declval<T>().serialize(std::declval<SerializerBase&>()), std::true_type());

    using result_type       = decltype(test<T>(0));
    static const bool value = result_type::value;
};

/// @brief A Helper to test if a class has a .deserialize(...) method that can take a
/// utility::Serializer object.
///
/// Usage: utility::has_deserialize<T>::value will return 1 if the method exists, 0 otherwise.
template<typename T>
struct has_deserialize
{
    // One of these two test method will exist, depending on if
    // std::declval<T>().deserialize(std::declval<utility::Serializer&>()) is valid.  If T contains
    // a serialize method that can take a utility::Serializer object, then it succeeds and the
    // return type becomes std::true_type, otherwise, the return type is std::false_type as fall
    // back. Relies on SPINAE in case you are not familiar.
    template<typename C>
    static constexpr auto test(...) -> std::false_type;
    template<typename C>
    static constexpr auto test(int)
      -> decltype(std::declval<T>().deserialize(std::declval<SerializerBase&>()), std::true_type());

    using result_type       = decltype(test<T>(0));
    static const bool value = result_type::value;
};

/// A fall back so that when all attempts of calling a serialize method/function fails, the
/// developer recieves a useful error.
template<typename SerializerType, typename T>
void serialize(SerializerType&, const T&)
{
    static_assert(false,
                  "A serialize function does not exist type T.  This probabely occurred when "
                  "type T was involved in a serialization but: 1) T is not a fundamental type, "
                  "2) T does not have a serialize method implemented, and "
                  "3) no specialization of utility::serialize() was implemented for T.  "
                  "One of these much be true to enable serialization.");
}

/// A fall back so that when all attempts of calling a deserialize method/function fails, the
/// developer recieves a useful error.
template<typename SerializerType, typename T>
void deserialize(SerializerType&, const T&)
{
    static_assert(false,
                  "A deserialize function does not exist type T.  This probabely occurred when "
                  "type T was involved in a deserialization but: 1) T is not a fundamental type, "
                  "2) T does not have a deserialize method implemented, and "
                  "3) no specialization of utility::deserialize() was implemented for T.  "
                  "One of these much be true to enable deserialization.");
}

/// std::array serialize(...) specialization
template<typename SerializerType, typename T, size_t Size>
void serialize(SerializerType& s, const std::array<T, Size>& arr)
{
    s(Slice<const T>(arr.data(), arr.size()));
}

/// std::array deserialize(...) specialization
template<typename SerializerType, typename T, size_t Size>
void deserialize(SerializerType& s, std::array<T, Size>& arr)
{
    s(Slice<T>(arr.data(), arr.size()));
}

/// std::vector serialize(...) specialization
template<typename SerializerType, typename T>
void serialize(SerializerType& s, const std::vector<T>& vec)
{
    s(vec.size(), Slice<const T>(vec.data(), vec.size()));
}

/// std::vector deserialize(...) specialization
template<typename SerializerType, typename T>
void deserialize(SerializerType& s, std::vector<T>& vec)
{
    std::size_t vec_size;
    s(vec_size);
    vec.resize(vec_size);
    s(Slice<T>(vec.data(), vec.size()));
}
}
