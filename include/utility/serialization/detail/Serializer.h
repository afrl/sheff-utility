#pragma once

#include "utility/Slice.h"
#include "utility/serialization/NamedValue.h"

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
    // Note: static_cast<void>(...) provided here in the very unlikely situation where the return
    // type has overloaded the comma operator.
    template<typename C>
    static constexpr auto test(int)
      -> decltype(static_cast<void>(std::declval<C>().serialize(std::declval<SerializerBase&>())),
                  std::true_type());

    using result_type       = decltype(test<T>(0));
    static const bool value = result_type::value;
};

/// @brief A Helper to test if a class has a .deserialize(...) method that can take a
/// utility::Serializer object.
///
/// Usage: utility::has_deserialize<T>::value will return 1 if the method exists, 0 otherwise.
template<class T>
struct has_deserialize
{
    // One of these two test method will exist, depending on if
    // std::declval<T>().deserialize(std::declval<utility::Serializer&>()) is valid.  If T contains
    // a deserialize method that can take a utility::Serializer object, then it succeeds and the
    // return type becomes std::true_type, otherwise, the return type is std::false_type as fall
    // back. Relies on SPINAE in case you are not familiar.
    template<typename C>
    static constexpr auto test(...) -> std::false_type;
    // Note: static_cast<void>(...) provided here in the very unlikely situation where the return
    // type has overloaded the comma operator.
    template<typename C>
    static constexpr auto test(int)
      -> decltype(static_cast<void>(std::declval<C>().deserialize(std::declval<SerializerBase&>())),
                  std::true_type());

    using result_type       = decltype(test<T>(0));
    static const bool value = result_type::value;
};

/// A fall back so that when all attempts of calling a serialize method/function fails, the
/// developer recieves a useful error.
template<typename SerializerType,
         typename T,
         typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
void serialize(SerializerType& s, const T& v)
{
    // If you received a compiler error here, then a serialize function does not exist for type T.
    // This probabely occurred when type T was involved in a serialization but:
    // 1) T is not a fundamental type,
    // 2) T does not have a serialize method implemented, and
    // 3) no specialization of utility::serialize() was implemented for T.
    // One of these much be true to enable serialization.
    v.does_not_have_serialize_method();
}

/// A fall back so that when all attempts of calling a deserialize method/function fails, the
/// developer recieves a useful error.
template<typename SerializerType,
         typename T,
         typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
void deserialize(SerializerType& s, const T& v)
{
    // If you received a compiler error here, then a deserialize function does not exist
    // for type T. This probabely occurred when type T was involved in a deserialization but:
    // 1) T is not a fundamental type,
    // 2) T does not have a deserialize method implemented, and
    // 3) no specialization of utility::deserialize() was implemented for T.
    // One of these much be true to enable deserialization.
    v.does_not_have_deserialize_method();
}

/// std::string serialize(...) specialization
template<typename SerializerType>
void serialize(SerializerType& s, const std::string& str)
{
    s(named_value("n_chars", str.size()));
    s(named_value("chars", Slice<const char>(str.data(), str.size())));
}

/// std::string deserialize(...) specialization
template<typename SerializerType>
void deserialize(SerializerType& s, std::string& str)
{
    std::string::size_type n_chars;
    s(n_chars);
    str.resize(n_chars);
    s(Slice<char>(str.data(), str.size()));
}

/// std::array serialize(...) specialization
template<typename SerializerType, typename T, size_t Size>
void serialize(SerializerType& s, const std::array<T, Size>& arr)
{
    s(named_value("data", Slice<const T>(arr.data(), arr.size())));
}

/// std::array deserialize(...) specialization
template<typename SerializerType, typename T, size_t Size>
void deserialize(SerializerType& s, std::array<T, Size>& arr)
{
    s(Slice<T>(arr.data(), arr.size()));
}

/// enum class serialize(...) specialization
template<typename SerializerType,
         typename T,
         typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr>
void serialize(SerializerType& s, const T& enum_value)
{
    using under_type = typename std::underlying_type<T>::type;
    s(reinterpret_cast<const under_type&>(enum_value));
}

/// enum class deserialize(...) specialization
template<typename SerializerType,
         typename T,
         typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr>
void deserialize(SerializerType& s, T& enum_value)
{
    using under_type = typename std::underlying_type<T>::type;
    s(reinterpret_cast<under_type&>(enum_value));
}

/// std::vector serialize(...) specialization
template<typename SerializerType, typename T>
void serialize(SerializerType& s, const std::vector<T>& vec)
{
    s(named_value("size", vec.size()), named_value("data", Slice<const T>(vec.data(), vec.size())));
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

/// utility::Slice<T> serialize(...) specialization for generic slices of types
template<typename SerializerType, typename T>
void serialize(SerializerType& s, const utility::Slice<const T>& slice)
{
    for (auto i : utility::range(slice.size()))
    {
        s(named_ref(std::to_string(i), slice[i]));
    }
}

/// utility::Slice<T> serialize(...) specialization for generic slices of types
template<typename SerializerType, typename T>
void serialize(SerializerType& s, const utility::Slice<T>& slice)
{
    for (auto i : utility::range(slice.size()))
    {
        s(named_ref(std::to_string(i), slice[i]));
    }
}

/// utility::Slice<T> deserialize(...) specialization for generic slices of types
/// Assumes the memory has already been allocated to an appropriate size.
template<typename SerializerType, typename T>
void deserialize(SerializerType& s, utility::Slice<T>& slice)
{
    for (auto& obj : slice)
    {
        s(obj);
    }
}

/// utility::Slice<std::array> serialize(...) specialization
/// Takes advantage of fact that a contiguous array of std::array objects will itself form a single
/// 1D continguous array.
template<typename SerializerType, typename T, size_t Size>
void serialize(SerializerType& s, const utility::Slice<const std::array<T, Size>>& slice_of_arr)
{
    s(named_value("data", Slice<const T>(slice_of_arr.data()->data(), slice_of_arr.size() * Size)));
}

/// utility::Slice<std::array> deserialize(...) specialization
template<typename SerializerType, typename T, size_t Size>
void deserialize(SerializerType& s, utility::Slice<std::array<T, Size>>& slice_of_arr)
{
    s(Slice<T>(slice_of_arr.data()->data(), slice_of_arr.size() * Size));
}
}
