#pragma once

#include "Exceptions.h"

#include <fstream>
#include <string>
#include <vector>

#define _EXCEPTION(m) _UTIL_MSG_EXCEPT(m)
namespace utility
{
namespace detail
{
template<typename ConvertToType, typename ConvertFromType, std::size_t Size, std::size_t... Is>
auto convert_array_impl(const std::array<ConvertFromType, Size>& src, std::index_sequence<Is...>)
{
    return std::array<ConvertToType, Size> { { static_cast<ConvertToType>(src[Is])... } };
}
} // end detail

/// Cross-platform fix for fopen: fopen gives security warning in MSVC
/// and requests fopen_s instead, but unavailable in the other compilers.
static FILE* file_open(std::string filename, std::string mode)
{
    FILE* in_file;
#if defined(_MSC_VER)
    errno_t err = fopen_s(&in_file, filename.c_str(), mode.c_str());
    if (err != 0)
    {
        throw _UTIL_MSG_EXCEPT(
          "File '" + filename +
          "' could not be opened, perhaps it is locked?  Error code: " + std::to_string(err));
    }
#else
    in_file = fopen(filename.c_str(), mode.c_str());
#endif
    return in_file;
}

/// @brief Static-stateless class providing helpers for reading/writing binary data to iostreams.
///
/// The output helpers in this class provides an interface to write data in the system's native data
/// type to a binary ostream. The input helpers in this class provides an interface to read data
/// from a type into datatypes that may be different from what wrote them.  This flexibility comes
/// at a computational cost of course.  The benefit would be that it allows files to be written
/// based on data structures with some datatypes and later read into new versions of the data
/// structures (where the types of members have been changed) or a new data structure altogether.
/// Furthermore, it does not currently check endian-ness, though that should be considered if
/// systems with other endian-ness for filesystem I/O will be used (like Solaris based systems for
/// example).
class IOUtils
{
  public:
    ///@{
    ///@name Output methods

    /// Writes a fundamental type to a binary file a the current stream position
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    static void binary_write(std::ostream& out, const T& val);

    /// Writes a vector of fundamental types to a binary file a the current stream position
    template<typename T,
             int Size,
             typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    static void binary_write(std::ostream& out, const std::array<T, Size>& val);

    /// Writes a vector of fundamental types to a binary file a the current stream position
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    static void binary_write(std::ostream& out, const std::vector<T>& val);

    /// Writes a slice of fundamental types to a binary file a the current stream position
    template<typename T,
             typename SizeType,
             typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    static void binary_write(std::ostream& out, const utility::Slice<T, SizeType>& val);

    /// Writes an array of a fundamental type to a binary file a the current stream position
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    static void binary_write(std::ostream& out, T* arr, const std::streamsize size);

    ///@}

    ///@{
    ///@name Input methods

    /// Reads a fundamental type from a binary file, but the type must match the type in the file.
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    static void binary_read(std::istream& in, T& val);

    /// Reads a integral type from a binary file at the current stream position
    /// Note: if type T does not have the same size as size_of_data_type_in_file
    /// then a conversion will occur.
    template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
    static void binary_read(std::istream& in, T& val, const int8_t size_of_data_type_in_file);

    /// Reads an array of a floating point type from a binary file at the current stream position
    /// Note: if type T does not have the same size as size_of_data_type_in_file
    /// then a conversion will occur.
    template<typename T,
             int Size,
             typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
    static void binary_read(std::istream& in,
                            std::array<T, Size>& val,
                            const int8_t size_of_data_type_in_file);

    template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
    static void binary_read(std::istream& in,
                            std::vector<T>& val,
                            const std::size_t num_values,
                            const int8_t size_of_data_type_in_file);

    template<typename T,
             typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
    static void binary_read(std::istream& in,
                            std::vector<T>& val,
                            const std::size_t num_values,
                            const int8_t size_of_data_type_in_file);

    template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
    static void binary_read(std::istream& in,
                            T* val,
                            const std::size_t num_values,
                            const int8_t size_of_data_type_in_file);

    template<typename T,
             typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
    static void binary_read(std::istream& in,
                            T* val,
                            const std::size_t num_values,
                            const int8_t size_of_data_type_in_file);

    ///@}

    ///@{
    ///@name Conversions

    template<typename ConvertToType, typename ConvertFromType, std::size_t Size>
    static auto convert_array(const std::array<ConvertFromType, Size>& src);

    ///@}
};


template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
inline void IOUtils::binary_write(std::ostream& out, const T& val)
{
    out.write(reinterpret_cast<const char*>(&val), sizeof(T));
}

template<typename T, int Size, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
inline void IOUtils::binary_write(std::ostream& out, const std::array<T, Size>& val)
{
    if (Size == 0)
        return;
    out.write(reinterpret_cast<const char*>(&val[0]),
              static_cast<std::streamsize>(Size * sizeof(T)));
}

template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
inline void IOUtils::binary_write(std::ostream& out, const std::vector<T>& v)
{
    if (v.size() == 0)
        return;
    out.write(reinterpret_cast<const char*>(&v[0]),
              static_cast<std::streamsize>(v.size() * sizeof(T)));
}

template<typename T,
         typename SizeType,
         typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
inline void IOUtils::binary_write(std::ostream& out, const utility::Slice<T, SizeType>& v)
{
    if (v.size() == 0)
    {
        return;
    }
    out.write(reinterpret_cast<const char*>(v.data()),
              static_cast<std::streamsize>(v.size() * sizeof(T)));
}

template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
inline void IOUtils::binary_write(std::ostream& out, T* arr, const std::streamsize size)
{
    if (size == 0)
        return;
    out.write(reinterpret_cast<const char*>(&arr[0]), size * sizeof(T));
}

template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
inline void IOUtils::binary_read(std::istream& in, T& val)
{
    in.read(reinterpret_cast<char*>(&val), sizeof(T));
}

#define BIN_READ_CONVERT(ReadType)                                         \
    else if (size_of_data_type_in_file == sizeof(ReadType))                \
    {                                                                      \
        ReadType tmp;                                                      \
        in.read(reinterpret_cast<char*>(&tmp), size_of_data_type_in_file); \
        val = static_cast<T>(tmp);                                         \
    }

#define BIN_READ_CONVERT_ARR(ReadType)                                  \
    else if (size_of_data_type_in_file == sizeof(ReadType))             \
    {                                                                   \
        std::array<ReadType, Size> tmp;                                 \
        in.read(reinterpret_cast<char*>(&tmp[0]),                       \
                static_cast<std::streamsize>(Size * sizeof(ReadType))); \
        val = convert_array<T>(tmp);                                    \
    }

template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type*>
inline void IOUtils::binary_read(std::istream& in, T& val, const int8_t size_of_data_type_in_file)
{
    if (size_of_data_type_in_file == sizeof(T))
    {
        in.read(reinterpret_cast<char*>(&val), size_of_data_type_in_file);
    }
    BIN_READ_CONVERT(int8_t)
    BIN_READ_CONVERT(int16_t)
    BIN_READ_CONVERT(int32_t)
    BIN_READ_CONVERT(int64_t)
    else throw _EXCEPTION("Conversion not available to data type with " +
                          std::to_string(size_of_data_type_in_file) + " bytes.");
}

template<typename T, int Size, typename std::enable_if<std::is_floating_point<T>::value, T>::type*>
inline void IOUtils::binary_read(std::istream& in,
                                 std::array<T, Size>& val,
                                 const int8_t size_of_data_type_in_file)
{
    if (size_of_data_type_in_file == sizeof(T))
    {
        in.read(reinterpret_cast<char*>(&val[0]),
                static_cast<std::streamsize>(val.size() * sizeof(T)));
    }
    BIN_READ_CONVERT_ARR(float)
    BIN_READ_CONVERT_ARR(double)
    else throw _EXCEPTION("Conversion not available to data type with " +
                          std::to_string(size_of_data_type_in_file) + " bytes.");
}

template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type*>
inline void IOUtils::binary_read(std::istream& in,
                                 std::vector<T>& val,
                                 const std::size_t num_values,
                                 const int8_t size_of_data_type_in_file)
{
    val.resize(num_values);
    if (size_of_data_type_in_file == sizeof(T))
    {
        in.read(reinterpret_cast<char*>(&val[0]),
                static_cast<std::streamsize>(num_values * sizeof(T)));
    }
    else if (size_of_data_type_in_file == sizeof(int16_t))
    {
        std::vector<int16_t> tmp(num_values);
        in.read(reinterpret_cast<char*>(&tmp[0]),
                static_cast<std::streamsize>(num_values * sizeof(int16_t)));
        val.assign(tmp.begin(), tmp.end());
    }
    else if (size_of_data_type_in_file == sizeof(int32_t))
    {
        std::vector<int32_t> tmp(num_values);
        in.read(reinterpret_cast<char*>(&tmp[0]),
                static_cast<std::streamsize>(num_values * sizeof(int32_t)));
        val.assign(tmp.begin(), tmp.end());
    }
    else
        throw _EXCEPTION("Conversion not available to data type with " +
                         std::to_string(size_of_data_type_in_file) + " bytes.");
}

template<typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type*>
inline void IOUtils::binary_read(std::istream& in,
                                 std::vector<T>& val,
                                 const std::size_t num_values,
                                 const int8_t size_of_data_type_in_file)
{
    val.resize(num_values);
    if (num_values == 0)
    {
        return;
    }
    else if (size_of_data_type_in_file == sizeof(T))
    {
        in.read(reinterpret_cast<char*>(&val[0]),
                static_cast<std::streamsize>(num_values * sizeof(T)));
    }
    else if (size_of_data_type_in_file == sizeof(float))
    {
        std::vector<float> tmp(num_values);
        in.read(reinterpret_cast<char*>(&tmp[0]),
                static_cast<std::streamsize>(num_values * sizeof(float)));
        val.assign(tmp.begin(), tmp.end());
    }
    else
        throw _EXCEPTION("Conversion not available to data type with " +
                         std::to_string(size_of_data_type_in_file) + " bytes.");
}

template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type*>
inline void IOUtils::binary_read(std::istream& in,
                                 T* val,
                                 const std::size_t num_values,
                                 const int8_t size_of_data_type_in_file)
{
    if (size_of_data_type_in_file == sizeof(T))
    {
        in.read(reinterpret_cast<char*>(&val[0]),
                static_cast<std::streamsize>(num_values * sizeof(T)));
    }
    else if (size_of_data_type_in_file == sizeof(int16_t))
    {
        std::vector<int16_t> tmp(num_values);
        in.read(reinterpret_cast<char*>(&tmp[0]),
                static_cast<std::streamsize>(num_values * sizeof(int16_t)));
        for (auto i = 0; i < num_values; i++)
            val[i] = static_cast<T>(tmp[i]);
    }
    else if (size_of_data_type_in_file == sizeof(int32_t))
    {
        std::vector<int32_t> tmp(num_values);
        in.read(reinterpret_cast<char*>(&tmp[0]),
                static_cast<std::streamsize>(num_values * sizeof(int32_t)));
        for (auto i = 0; i < num_values; i++)
            val[i] = static_cast<T>(tmp[i]);
    }
    else
        throw _EXCEPTION("Conversion not available to data type with " +
                         std::to_string(size_of_data_type_in_file) + " bytes.");
}

template<typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type*>
inline void IOUtils::binary_read(std::istream& in,
                                 T* val,
                                 const std::size_t num_values,
                                 const int8_t size_of_data_type_in_file)
{
    if (size_of_data_type_in_file == sizeof(T))
    {
        in.read(reinterpret_cast<char*>(&val[0]),
                static_cast<std::streamsize>(num_values * sizeof(T)));
    }
    else if (size_of_data_type_in_file == sizeof(float))
    {
        std::vector<float> tmp(num_values);
        in.read(reinterpret_cast<char*>(&tmp[0]),
                static_cast<std::streamsize>(num_values * sizeof(float)));
        for (auto i = 0; i < num_values; i++)
            val[i] = static_cast<T>(tmp[i]);
    }
    else
        throw _EXCEPTION("Conversion not available to data type with " +
                         std::to_string(size_of_data_type_in_file) + " bytes.");
}

template<typename ConvertToType, typename ConvertFromType, std::size_t Size>
inline auto IOUtils::convert_array(const std::array<ConvertFromType, Size>& src)
{
    return detail::convert_array_impl<ConvertToType>(src, std::make_index_sequence<Size>());
}

#undef BIN_READ_CONVERT
#undef BIN_READ_CONVERT_ARR
}
#undef _EXCEPTION
