#pragma once

#include "Exceptions.h"
#include "Hash.h"

#include <assert.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

namespace utility
{

namespace detail
{
struct ProfileStart
{
    uint64_t id;
    std::string caller;
    std::string description;
    std::chrono::high_resolution_clock::time_point start_time;
    int64_t start_physical_memory;
    int64_t start_virtual_memory;
    uint64_t depth;
};

struct ProfileInterval
{
    uint64_t id;
    uint64_t parent_id;
    std::string caller;
    std::string description;
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    int64_t start_physical_memory;
    int64_t end_physical_memory;
    int64_t start_virtual_memory;
    int64_t end_virtual_memory;
    uint64_t depth;
};
}

// Macros to provide conveniently mark profiled sections
#define _F_BEGIN()       utility::Profile::start(__func__);
#define _F_BEGIN2(descr) utility::Profile::start(__func__, descr);
#define _F_END()         utility::Profile::end(__func__);

/// Converts time to a pretty (human readable) string
std::string beautify_duration(std::chrono::milliseconds input_seconds);

/// Converts memory useage to a pretty (human readable) string
std::string beautify_memory(const uint64_t& mem);

class Profile
{
  public:
    ~Profile();

    /// @brief Starts writing a log "profile.csv" file
    static void output_log();

    /// @brief Begins a new profiling section
    ///
    /// @param caller - name of the function that the section is within
    /// @param description - an optional description of the profiled section that will appear in the
    ///        printed profile overview when print() is called.
    static void start(const char* caller, const std::string& description = "");

    /// @brief Ends the last profiling section.
    ///
    /// @param caller - name of the function that the section is within (should match the caller of
    ///        the current section being profiled)
    /// @returns The time elapsed for the section.
    static std::chrono::milliseconds end(const char* caller);

    /// @brief Prints an overview of profiled sections (should be called after all sections have
    /// been closed)
    ///
    /// @param combined_same_calls - if true, then all calls with the same caller, description, and
    ///        call path will be combined into one line showing total and average instead of a line
    ///        for each call.
    static void print(bool combined_same_calls = false);

    /// @brief Suppresses printing profiling info when the static singleton is destructed.
    static void set_quiet(const bool is_quiet);

    /// @brief Sets the rank of this process to avoid multiple ranks printing information.
    static void set_rank(const int rank);

    /// @brief Resets all profiling information.  Be sure not to call this while
    /// a profiled interval is going.
    static void reset();

  private:
    // The current list of ongoing intervals that were started but not stopped
    std::deque<detail::ProfileStart> starts_;
    // Completed sections marked with start/stop for timing
    std::deque<detail::ProfileInterval> completed_intervals_;
    // A counter for the number of total intervals started
    uint64_t counter_ = 0;
    // Current depth of profiled calls
    uint64_t depth_ = 0;
    // If true, then profiling information will only be printed when explicitly called (not on
    // destruction)
    bool is_quiet_ = true;
    // Note: MPIUtils might be destructed before Profile, so the rank must be stored
    int rank_ = 0;
    // The number of spaces to indent each new depth for the messages
    int msg_pad_size_ = 3;
    // Indicates whether log_file_ should be written to
    bool write_log_ = false;
    // Log file that will be updated with every call to end()
    std::ofstream log_file_;

    /// @brief Accesses the singleton instance for this process.
    static Profile& get();

    Profile() = default;

    /// Builds an ordered list of messages to print profiling information to the screen (does not
    /// combine calls to the same function)
    void build_prof_messages(std::vector<std::string>& messages, uint64_t& max_line_length_needed);

    /// Builds an ordered list of messages to print profiling information to the screen (combines
    /// calls to the same function)
    void build_unique_prof_messages(std::vector<std::string>& messages,
                                    uint64_t& max_line_length_needed);

    /// @brief Print kernel for the profile overivew.
    void print_kernel(bool combined_same_calls = false) noexcept;
};
}