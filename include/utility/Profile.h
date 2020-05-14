#pragma once

#include "Exceptions.h"

#include <chrono>
#include <iostream>
#include <stack>
#include <string>

namespace utility
{
    namespace detail
    {
        struct ProfileStart
        {
            const int id;
            const std::string caller;
            const std::string description;
            const std::chrono::high_resolution_clock::time_point start_time;
            const int depth;
        };

        struct ProfileInterval
        {
            const int id;
            const std::string caller;
            const std::string description;
            const std::chrono::high_resolution_clock::time_point start_time;
            const std::chrono::high_resolution_clock::time_point end_time;
            const int depth;
        };
    }

#define _F_BEGIN() utility::Profile::start(__func__);
#define _F_BEGIN2(descr) utility::Profile::start(__func__, descr);
#define _F_END() utility::Profile::end(__func__);

    class Profile
    {
    private:
        static Profile& get()
        {
            static Profile singleton;
            return singleton;
        }

        Profile()
        {
            counter_ = 0;
            depth_ = 0;
        }

        static std::string beautify_duration(std::chrono::milliseconds input_seconds)
        {
            using namespace std::chrono;
            auto h = duration_cast<hours>(input_seconds);
            input_seconds -= h;
            auto m = duration_cast<minutes>(input_seconds);
            input_seconds -= m;
            auto s = duration_cast<seconds>(input_seconds);
            input_seconds -= s;
            auto ms = duration_cast<milliseconds>(input_seconds);

            auto hc = h.count();
            auto mc = m.count();
            auto sc = s.count();
            auto msc = ms.count();

            std::stringstream ss;
            ss.fill('0');
            if (hc) {
                ss << hc << "h";
            }
            if (hc || mc)
            {
                if (hc) { ss << " "; }
                ss << mc << "m";
            }
            if (hc || mc || sc)
            {
                if (hc || mc) { ss << " "; }
                ss << sc << 's';
            }
            if (hc || mc || sc || msc)
            {
                if (hc || mc || sc) { ss << " "; }
                ss << msc << "ms";
            }
            if (!hc && !mc && !sc && !msc)
            {
                ss << "-";
            }

            return ss.str();
        }

    public:
        ~Profile()
        {
            print();
        }

        static void start(
            const char* caller,
            const std::string& description = "")
        {
            int depth = get().starts_.size();
            
            get().starts_.push_back({
                    get().counter_++,
                    demangle(caller),
                    description,
                    std::chrono::high_resolution_clock::now(),
                    depth
                });
        }

        static void end(
            const char* caller)
        {
            auto popped_data = get().starts_.back();
            get().starts_.pop_back();

            assert(demangle(caller) == popped_data.caller);

            auto end_time = std::chrono::high_resolution_clock::now();

            get().completed_intervals_.push_front({
                    popped_data.id,
                    popped_data.caller,
                    popped_data.description,
                    popped_data.start_time,
                    end_time,
                    popped_data.depth
                });
        }

        void print()
        {
            const int pad_size = 3;

            // This should always be empty, if not, check the code at whatever function is in starts_
            assert(starts_.empty());

            std::vector<std::string> msg_by_id(completed_intervals_.size());
            int curr_parent_id = -1;
            int max_line_length = 0;
            for (auto& interval : completed_intervals_)
            {
                std::string curr_pad(interval.depth * pad_size, ' ');

                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    interval.end_time - interval.start_time);

                std::string msg = curr_pad;
                auto duration_str = beautify_duration(duration);
                msg += interval.caller;
                if (interval.description != "")
                {
                    msg += " (" + interval.description + ")";
                }
                msg += ": $ " + duration_str; // Note: use $ for pad marker, to be replaced later

                msg_by_id[interval.id] = msg;

                if (msg.size() > max_line_length)
                {
                    max_line_length = msg.size();
                }

                curr_parent_id = interval.id;
            }
            max_line_length += 2;

            std::cout << std::endl << std::string(max_line_length, '-') << std::endl;
            std::cout << "Profiling results (coarse):" << std::endl << std::endl;
            for (auto msg : msg_by_id)
            {
                auto find_ind = msg.find_last_of('$');
                auto pad_size = max_line_length - msg.size();
                std::cout << msg.substr(0, find_ind) << std::string(pad_size, ' ') << msg.substr(find_ind + 1) << std::endl;
            }
            std::cout << std::string(max_line_length, '-') << std::endl;
        }

    private:
        std::deque<detail::ProfileStart> starts_;
        std::deque<detail::ProfileInterval> completed_intervals_;
        int counter_;
        int depth_;
    };
}