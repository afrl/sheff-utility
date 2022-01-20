#pragma once

#include "Serializer.h"
#include "nlohmann/json.hpp"
#include "utility/Slice.h"
#include "utility/SliceList.h"
#include "utility/StringUtils.h"

#include <deque>
#include <ostream>

namespace utility
{
/// @brief A class that writes a binary serialization to an std::ostream while tracking the location
/// of every object in the binray stream, which can be used to jump to a specific piece of data.
///
/// This class follows the pattern of serialization pattern this project uses.
/// Refer to serialization.md for a summary of the overall strategy.
class LocatableBinarySerializer : public Serializer<LocatableBinarySerializer>
{
  public:
    using json = nlohmann::json;

    LocatableBinarySerializer(std::ostream& lookup_table_out, std::ostream& binary_out)
      : lookup_table_out_(lookup_table_out)
      , binary_out_(binary_out)
    {
        location_stack_.push_back(std::ref(j_));
    }

    ~LocatableBinarySerializer()
    {
        if (!dumped_lookup_table_)
        {
            lookup_table_out_ << j_.dump(4) << std::endl;
        }
    }

    /// @brief Get the JSON object for the lookup table.
    const json& get_json_lookup_table() const
    {
        return j_;
    }

    /// @brief Forces output of the lookup table to the stream provided to the constructor.
    void write_lookup_table()
    {
        lookup_table_out_ << j_.dump(4) << std::endl;
        dumped_lookup_table_ = true;
    }

    /// @brief Resets the state of the serializer (throwing away the lookup table).
    void reset()
    {
        j_ = json::object();
        location_stack_.clear();
        location_stack_.push_back(std::ref(j_));
        dumped_lookup_table_ = false;
    }

  private:
    /// Reference to the ostream given to the constructor to which the binary
    /// data will be written as the operator() is called.
    std::ostream& binary_out_;
    /// Reference to the ostream given to which the JSON lookup table will be written when the
    /// destructor or write_lookup_table is called.
    std::ostream& lookup_table_out_;
    bool dumped_lookup_table_ = false;
    json j_;
    std::deque<std::reference_wrapper<json>> location_stack_;

    /// Grant access to parent class so it can call process(...)
    friend class Serializer<LocatableBinarySerializer>;

    /// Serializes fundamental types.
    /// @param v - a single object of a fundamental datatype to write
    /// @param name - (ignored for this serializer type)
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const T& v, const std::string& name = "")
    {
        location_stack_.back().get()[name] = (long long)binary_out_.tellp();
        binary_out_.write(reinterpret_cast<const char*>(&v), sizeof(T));
    }

    /// Serializes a slice for fundamental types (int, double, etc.)
    /// @param name - name of value in the JSON lookup table
    template<typename T,
             typename SizeType,
             typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const Slice<T, SizeType>& slice, const std::string& name = "")
    {
        location_stack_.back().get()[name] = (long long)binary_out_.tellp();
        binary_out_.write(reinterpret_cast<const char*>(slice.data()), sizeof(T) * slice.size());
    }

    /// Serializes a slice for non-fundamental types
    /// @param name - name of value in the JSON
    template<typename T,
             typename SizeType,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const Slice<T, SizeType>& slice, const std::string& name = "")
    {
        json arr = json::array();
        for (auto& v : slice)
        {
            arr.push_back(json::object());
            location_stack_.push_back(std::ref(arr.back()));
            Serializer<LocatableBinarySerializer>::forward_to_serialize(v);
            location_stack_.pop_back();
        }
        location_stack_.back().get()[name] = arr;
    }

    /// The fall back for non-fundamental types.  This passes the argument on to the
    /// Serializer::forward_to_serialize method, which will find the appropriate serialize
    /// method/function and call process() on the fundamental types within any structures.
    /// @param v - an object of a non-fundamental type that will be forwarded to serialize
    /// @param name - (ignored for this serializer type)
    template<typename T,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const T& v, const std::string& name = "")
    {
        location_stack_.back().get()[name] = json::object();
        location_stack_.back().get()[name]["binary_location"] = (long long)binary_out_.tellp();
        location_stack_.push_back(std::ref(location_stack_.back().get()[name]));
        Serializer<LocatableBinarySerializer>::forward_to_serialize(v);
        location_stack_.pop_back();
    }
};
} // namespace utility
