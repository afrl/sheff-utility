#include "utility/Profile.h"

#include <fstream>

#ifdef WIN32
// clang-format off
#include "windows.h"
#include "psapi.h"
// clang-format on
#endif

namespace utility
{

#ifdef WIN32
int64_t query_virtual_memory()
{
    _PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T virtual_memory_used = pmc.PrivateUsage;
    return virtual_memory_used;
}

int64_t query_physical_memory()
{
    _PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T physical_memory_used = pmc.WorkingSetSize;
    return physical_memory_used;
}
#else
int64_t query_virtual_memory()
{
    return 0.0;
}

int64_t query_physical_memory()
{
    return 0.0;
}
#endif

std::string beautify_duration(std::chrono::milliseconds input_seconds)
{
    using namespace std::chrono;
    auto h = duration_cast<hours>(input_seconds);
    input_seconds -= h;
    auto m = duration_cast<minutes>(input_seconds);
    input_seconds -= m;
    auto s = duration_cast<seconds>(input_seconds);
    input_seconds -= s;
    auto ms = duration_cast<milliseconds>(input_seconds);

    auto hc  = h.count();
    auto mc  = m.count();
    auto sc  = s.count();
    auto msc = ms.count();

    std::stringstream ss;
    ss.fill('0');
    if (hc)
    {
        ss << hc << "h";
    }
    if (hc || mc)
    {
        if (hc)
        {
            ss << " ";
        }
        ss << mc << "m";
    }
    if (hc || mc || sc)
    {
        if (hc || mc)
        {
            ss << " ";
        }
        ss << sc << 's';
    }
    if (hc || mc || sc || msc)
    {
        if (hc || mc || sc)
        {
            ss << " ";
        }
        ss << msc << "ms";
    }
    if (!hc && !mc && !sc && !msc)
    {
        ss << "-";
    }

    return ss.str();
}

std::string beautify_memory(const uint64_t& mem)
{
    static const std::vector<std::string> byte_mag_names = {
        "bytes", "KB", "MB", "GB", "TB", "PB"
    };

    int byte_mag = 0;
    while (mem / (pow(1028, byte_mag + 1)) > 1 && byte_mag < byte_mag_names.size() - 1)
    {
        byte_mag++;
    }

    double mem_fraction = (double)mem / pow(1028.0, (double)byte_mag);
    return std::to_string(mem_fraction) + " " + byte_mag_names[byte_mag];
}

Profile::~Profile()
{
    if (rank_ == 0 && !is_quiet_ && completed_intervals_.size() > 0)
    {
        print_kernel();
    }
    if (get().write_log_)
    {
        get().log_file_.close();
    }
}

void Profile::output_log()
{
    get().write_log_ = true;
    get().log_file_.open("profile.csv");
    get().log_file_ << "caller,time (ms),delta_physical_ram (bytes),delta_virtual_ram (bytes),total_ram (bytes)" << std::endl;
}

void Profile::start(const char* caller, const std::string& description)
{
    get().starts_.push_back({ get().counter_++,
                              demangle(caller),
                              description,
                              std::chrono::high_resolution_clock::now(),
                              query_physical_memory(),
                              query_virtual_memory(),
                              get().starts_.size() });
}

std::chrono::milliseconds Profile::end(const char* caller)
{
    auto popped_data = get().starts_.back();
    get().starts_.pop_back();

    uint64_t parent_id = (std::numeric_limits<uint64_t>::max)();
    if (!get().starts_.empty())
    {
        auto& parent_data = get().starts_.back();
        parent_id         = parent_data.id;
    }

    assert(demangle(caller) == popped_data.caller);

    auto end_time = std::chrono::high_resolution_clock::now();

    get().completed_intervals_.push_front({ popped_data.id,
                                            parent_id,
                                            popped_data.caller,
                                            popped_data.description,
                                            popped_data.start_time,
                                            end_time,
                                            popped_data.start_physical_memory,
                                            query_physical_memory(),
                                            popped_data.start_virtual_memory,
                                            query_virtual_memory(),
                                            popped_data.depth });

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      get().completed_intervals_.front().end_time - get().completed_intervals_.front().start_time);

    if (get().write_log_)
    {
        auto& interval = get().completed_intervals_.front();
        get().log_file_
          << interval.caller + "," + std::to_string(duration.count()) + "," +
               std::to_string(interval.end_physical_memory - interval.start_physical_memory) + "," +
               std::to_string(interval.end_virtual_memory - interval.start_virtual_memory) + "," +
               std::to_string(interval.end_physical_memory + interval.end_virtual_memory) << std::endl;
    }

    return duration;
}

void Profile::print(bool combined_same_calls)
{
    get().print_kernel(combined_same_calls);
}

void Profile::set_quiet(const bool is_quiet)
{
    get().is_quiet_ = is_quiet;
}

void Profile::set_rank(const int rank)
{
    get().rank_ = rank;
}

void Profile::reset()
{
    if (!get().starts_.empty())
    {
        throw _UTIL_MSG_EXCEPT("Profiling cannot be reset while an interval is ongoing.");
    }

    get().completed_intervals_.clear();
    get().counter_   = 0;
    get().depth_     = 0;
    get().write_log_ = false;
    get().log_file_.close();
}



/// @brief Accesses the singleton instance for this process.

inline Profile& Profile::get()
{
    static Profile singleton;
    return singleton;
}

void Profile::build_prof_messages(std::vector<std::string>& messages,
                                  uint64_t& max_line_length_needed)
{
    messages.resize(completed_intervals_.size());
    max_line_length_needed = 0;
    for (auto& interval : completed_intervals_)
    {
        std::string curr_pad(interval.depth * msg_pad_size_, ' ');

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(interval.end_time -
                                                                              interval.start_time);

        std::string msg   = curr_pad;
        auto duration_str = beautify_duration(duration);
        msg += interval.caller;
        if (interval.description != "")
        {
            msg += " (" + interval.description + ")";
        }
        msg += ": $ " + duration_str; // Note: use $ for pad marker, to be replaced later

        messages.at(interval.id) = msg;

        if (msg.size() > max_line_length_needed)
        {
            max_line_length_needed = msg.size();
        }
    }
    max_line_length_needed += 2;
}

void Profile::build_unique_prof_messages(std::vector<std::string>& messages,
                                         uint64_t& max_line_length_needed)
{
    // Create a sorted version of the intervals such that the id matches the index (useful for
    // temporary book-keeping).
    auto sorted_intervals = completed_intervals_;
    std::sort(sorted_intervals.begin(),
              sorted_intervals.end(),
              [](const detail::ProfileInterval& a, const detail::ProfileInterval& b) -> bool
              { return a.id < b.id; });

    // Create a map from the caller + description to a unique id
    std::map<std::string, uint64_t> caller_descr_to_id;
    std::vector<uint64_t> sorted_interval_index_to_id;
    for (auto& interval : sorted_intervals)
    {
        const std::string call_descr = interval.caller + "_" + interval.description;

        auto find_iter = caller_descr_to_id.find(call_descr);
        if (find_iter == caller_descr_to_id.end())
        {
            auto next_id                   = caller_descr_to_id.size();
            caller_descr_to_id[call_descr] = next_id;
            sorted_interval_index_to_id.push_back(next_id);
        }
        else
        {
            sorted_interval_index_to_id.push_back(find_iter->second);
        }
    }

    // Group intervals by the call path
    std::map<uint64_t, uint64_t> call_path_to_call_path_id_map;
    std::vector<std::size_t> sorted_interval_index_to_call_path_id;
    std::vector<std::vector<uint64_t>> call_path_to_intervals_map;
    for (auto& interval : sorted_intervals)
    {
        std::vector<uint64_t> call_path_ids = { sorted_interval_index_to_id[interval.id] };
        auto curr_index                     = interval.parent_id;
        while (curr_index != (std::numeric_limits<decltype(curr_index)>::max)())
        {
            auto& unique_id = sorted_interval_index_to_id[curr_index];
            call_path_ids.push_back(unique_id);
            curr_index = sorted_intervals[curr_index].parent_id;
        }

        auto hash      = utility::hash(call_path_ids);
        auto find_iter = call_path_to_call_path_id_map.find(hash);
        if (find_iter == call_path_to_call_path_id_map.end())
        {
            auto next_id                        = call_path_to_call_path_id_map.size();
            call_path_to_call_path_id_map[hash] = next_id;
            sorted_interval_index_to_call_path_id.push_back(next_id);
            call_path_to_intervals_map.push_back({ interval.id });
        }
        else
        {
            sorted_interval_index_to_call_path_id.push_back(find_iter->second);
            call_path_to_intervals_map[find_iter->second].push_back(interval.id);
        }
    }

    // Create messages
    messages.clear();
    max_line_length_needed = 0;
    std::set<uint64_t> processed_call_paths;
    for (auto& interval_indices : call_path_to_intervals_map)
    {
        assert(interval_indices.size() > 0);
        auto& interval = sorted_intervals[interval_indices.front()];

        // Calculate the avg time required for this particular call (with this call path)
        std::chrono::milliseconds avg_duration   = std::chrono::milliseconds::zero();
        std::chrono::milliseconds total_duration = std::chrono::milliseconds::zero();
        for (auto& interval_id : interval_indices)
        {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
              sorted_intervals[interval_id].end_time - sorted_intervals[interval_id].start_time);
            total_duration += duration;
        }
        avg_duration = total_duration / interval_indices.size();


        std::string curr_pad(interval.depth * msg_pad_size_, ' ');
        std::string msg = curr_pad;
        msg += interval.caller;
        if (interval.description != "")
        {
            msg += " (" + interval.description + ")";
        }
        msg += ": $ " + beautify_duration(
                          total_duration); // Note: use $ for pad marker, to be replaced later
        if (interval_indices.size() > 1)
        {
            msg += " (avg " + beautify_duration(avg_duration) + " for " +
                   std::to_string(interval_indices.size()) + " calls)";
        }
        messages.push_back(msg);


        if (msg.size() > max_line_length_needed)
        {
            max_line_length_needed = msg.size();
        }
    }
    max_line_length_needed += 6;
}

void Profile::print_kernel(bool combined_same_calls) noexcept
{
    constexpr int pad_size = 3;

    // Catch all errors and do nothing to satisfy "noexcept"
    try
    {
        // This should always be empty, if not, check the code at whatever function is in
        // starts_
        if (!starts_.empty())
        {
            std::cout << std::endl
                      << "Profile information cannot be printed because something went wrong."
                      << std::endl;
            return;
        }

        std::vector<std::string> messages;
        uint64_t max_line_length = 0;
        if (combined_same_calls)
        {
            build_unique_prof_messages(messages, max_line_length);
        }
        else
        {
            build_prof_messages(messages, max_line_length);
        }

        std::cout << std::endl << std::string(max_line_length, '-') << std::endl;
        std::cout << "Profiling results (coarse):" << std::endl << std::endl;
        for (auto msg : messages)
        {
            const auto find_ind     = msg.find_last_of('$');
            const auto msg_pad_size = max_line_length - msg.size();
            std::cout << msg.substr(0, find_ind) << std::string(msg_pad_size, ' ')
                      << msg.substr(find_ind + 1) << std::endl;
        }
        std::cout << std::string(max_line_length, '-') << std::endl;
    }
    catch (...)
    {
    }
}


}