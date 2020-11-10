#pragma once

#include "Serializer.h"
#include "nlohmann/json.hpp"

#include <deque>

namespace utility
{
/// @brief A class that counts the bytes that a serialization of the data would have.
///
/// This class follows the pattern of serialization pattern this project uses.
/// Refer to serialization.md for a summary of the overall strategy.
class JSONSerializer : public Serializer<JSONSerializer>
{
  public:
    using json = nlohmann::json;

    JSONSerializer(std::ostream& out)
      : out_(out)
      , dumped_to_out_(false)
    {
        object_stack_.push_back(std::ref(j_));
    }

    ~JSONSerializer()
    {
        if (!dumped_to_out_)
        {
            out_ << j_.dump(4) << std::endl;
        }
    }

    const json& json_object() const
    {
        return j_;
    }

    void write_out()
    {
        out_ << j_.dump(4) << std::endl;
        dumped_to_out_ = true;
    }

    void reset()
    {
        j_ = json::object();
        object_stack_.clear();
        object_stack_.push_back(std::ref(j_));
        dumped_to_out_ = false;
    }

  private:
    std::ostream& out_;
    bool dumped_to_out_;
    json j_;
    std::deque<std::reference_wrapper<json>> object_stack_;

    /// Grant access to parent class so it can call process(...)
    friend class Serializer<JSONSerializer>;

    /// Counts the number of bytes for fundamental types.
    /// @param name - name of value in the JSON, if left empty a default sequence of names value1,
    /// value2, ... will be used
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const T& v, const std::string& name = "")
    {
        object_stack_.back().get()[name] = v;
    }

    /// Counts the number of bytes for a slice.
    /// @param name - name of value in the JSON, if left empty a default sequence of names value1,
    /// value2, ... will be used
    template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const Slice<T>& slice, const std::string& name = "")
    {
        json arr = json::array();
        for (auto& v : slice)
        {
            arr.push_back(v);
        }
        object_stack_.back().get()[name] = arr;
    }

    /// The fall back for non-fundamental types.  This passes the argument on to the
    /// Serializer::forward_to_serialize method, which will find the appropriate serialize
    /// method/function and call process() on the fundamental types within any structures.
    /// @param name - name of value in the JSON, if left empty a default sequence of names value1,
    /// value2, ... will be used
    template<typename T,
             typename std::enable_if<!std::is_fundamental<T>::value, T>::type* = nullptr>
    inline void process(const T& v, const std::string& name = "")
    {
        object_stack_.back().get()[name] = json::object();
        object_stack_.push_back(std::ref(object_stack_.back().get()[name]));
        Serializer<JSONSerializer>::forward_to_serialize(v);
        object_stack_.pop_back();
    }
};

template<typename... Args>
static nlohmann::json to_json(Args&&... args)
{
    std::stringstream ss;
    JSONSerializer json_serializer(ss);
    json_serializer(std::forward<Args>(args)...);
    return json_serializer.json_object();
}

}
