#pragma once

#include "LocatableBinarySerializer.h"
#include "SerialByteCounter.h"
#include "utility/Exceptions.h"
#include "utility/Filesystem.h"
#include "utility/MPIUtils.h"
#include "utility/serialization/NamedValue.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

namespace utility
{
using json = nlohmann::json;

namespace detail
{
/// @brief Helper struct for serialization I/O that stores the offsets in the binary data for each
/// rank.
struct RankOffsets
{
    int n_ranks;
    std::vector<std::streamoff> offsets;

    static void to_json(json& j, const RankOffsets& info)
    {
        j = json { { "n_ranks", info.n_ranks }, { "offsets", info.offsets } };
    }

    static void from_json(const json& j, RankOffsets& info)
    {
        j.at("n_ranks").get_to(info.n_ranks);
        j.at("offsets").get_to(info.offsets);
    }
};
}

template<typename... Args>
void save(const std::string filestem, Args&&... args)
{
    // Strip an extension if one was given
    auto filestem_fs = fs::path(filestem);
    std::string filestem_sanitized =
      (filestem_fs.parent_path() / filestem_fs.stem()).generic_string();
    std::string binary_path = filestem_sanitized + ".bin";
    std::string json_path   = filestem_sanitized + ".json";

    // Count how many bytes will be needed for the serialized data by rank
    utility::SerialByteCounter byte_counter;
    byte_counter(std::forward<Args>(args)...);
    std::size_t n_bytes = byte_counter.n_bytes();
    auto size_by_rank   = utility::MPIUtils::gather_vectors<std::size_t>({ n_bytes });

    detail::RankOffsets rank_offsets;
    if (utility::MPIUtils::is_root())
    {
        rank_offsets.n_ranks       = utility::MPIUtils::get_num_local_ranks();
        std::streamoff size_needed = 0;
        for (auto& s : size_by_rank)
        {
            rank_offsets.offsets.push_back(size_needed);
            size_needed += s;
        }

        // For now, zeroes will be written to a file to preallocate it.
        // the OS should have a way to preallocate a non-fragmented file
        // without writing the data, but it is not clear how to do it in a
        // portable way.  So for now...the dumb method will suffice.
        std::ofstream bin_stream(binary_path, std::ios::binary);
        const int buffer_size = 1024 * 1024; // 1 MB buffer
        std::vector<char> zero_buffer(buffer_size, 0);
        std::streamoff num_bytes_left = size_needed;
        while (num_bytes_left != 0)
        {
            if (num_bytes_left < buffer_size)
            {
                bin_stream.write(&zero_buffer[0], num_bytes_left);
                num_bytes_left = 0;
            }
            else
            {
                bin_stream.write(&zero_buffer[0], buffer_size);
                num_bytes_left -= buffer_size;
            }
        }
        bin_stream.close();
    }
    // Be sure the root rank finishes writing the file before opening it
    utility::MPIUtils::barrier();

    auto offset = utility::MPIUtils::scatter(rank_offsets.offsets);

    // Everyone needs to open the file in in/out mode to be able to seek to different parts and
    // write at will. Note: append mode will not let you go back before the end.
    std::ofstream out_stream;
    out_stream.open(binary_path, std::ios::binary | std::fstream::in | std::fstream::out);
    out_stream.seekp(offset, std::ios::beg);

    // Let each rank serialize their bvp_data
    std::stringstream lookup_table_ss;
    utility::LocatableBinarySerializer s(lookup_table_ss, out_stream);
    s(std::forward<Args>(args)...);
    s.write_lookup_table();

    out_stream.close();

    // Write the JSON file for the meta information
    auto lookup_table_strs_by_rank =
      utility::MPIUtils::gather_strings_by_rank(lookup_table_ss.str());
    if (utility::MPIUtils::is_root())
    {
        json j;

        j["n_ranks"]               = rank_offsets.n_ranks;
        j["lookup_tables_by_rank"] = json::array();
        auto& lookup_tables        = j["lookup_tables_by_rank"];
        for (auto rank : utility::indices(lookup_table_strs_by_rank))
        {
            lookup_tables.push_back(json::object());
            lookup_tables.back()                = json::parse(lookup_table_strs_by_rank[rank]);
            lookup_tables.back()["rank_offset"] = rank_offsets.offsets[rank];
        }

        std::ofstream json_stream(json_path);
        json_stream << j;
        json_stream.close();
    }
}

template<typename T>
void load(const std::string filestem,
          const int& rank_to_load,
          const std::vector<std::string>& data_location,
          T& t)
{
    // Strip an extension if one was given
    auto filestem_fs = fs::path(filestem);
    std::string filestem_sanitized =
      (filestem_fs.parent_path() / filestem_fs.stem()).generic_string();
    std::string binary_path = filestem_sanitized + ".bin";
    std::string json_path   = filestem_sanitized + ".json";

    _UTIL_ASSERT(fs::exists(binary_path),
                 "The binary part of the file \"" + binary_path + "\" did not exist.");
    _UTIL_ASSERT(fs::exists(json_path),
                 "The JSON part of the file \"" + json_path + "\" did not exist.");

    json j;
    std::ifstream json_in(json_path);
    json_in >> j;

    _UTIL_ASSERT(rank_to_load >= 0, "The rank to load must be greater than 0.");
    _UTIL_ASSERT(rank_to_load < j["lookup_tables_by_rank"].size(),
                 "The rank to load was larger than the number of ranks that wrote the data file.");

    std::size_t rank_offset = j["lookup_tables_by_rank"][rank_to_load]["rank_offset"];

    std::reference_wrapper<json> curr_j_obj = j["lookup_tables_by_rank"][rank_to_load];
    for (auto& loc : data_location)
    {
        _UTIL_ASSERT(curr_j_obj.get().find(loc) != curr_j_obj.get().end(),
                     "The JSON path was not valid.  The value '" + loc +
                       "' was not found in current JSON object:\n" + curr_j_obj.get().dump());
        curr_j_obj = curr_j_obj.get()[loc];
    }
    _UTIL_ASSERT(curr_j_obj.get().is_number_integer() ||
                   curr_j_obj.get().find("binary_location") != curr_j_obj.get().end(),
                 "The JSON lookup table seems to be incorrect.  The data path given does not refer "
                 "to an integer or an object with a value for 'binary_location'.");
    std::size_t data_location_in_rank = 0;
    if (!curr_j_obj.get().is_number_integer())
    {
        data_location_in_rank = curr_j_obj.get()["binary_location"];
    }
    else
    {
        data_location_in_rank = curr_j_obj.get();
    }

    std::ifstream bin_in(binary_path, std::ios::in | std::ios::binary);
    bin_in.seekg(rank_offset + data_location_in_rank);

    utility::InputDeserializer s(bin_in);

    s(t);

    bin_in.close();
}
}