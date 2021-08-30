/// Unit test for Hash.hw

#include "utility/MPIUtils.h"

#include <assert.h>
#include <iostream>

int main(int argc, char* argv[])
{
    utility::MPIUtils::init(&argc, &argv);

    // Test MPIUtils::scatter_arrays
    {
        std::vector<int> data_to_scatter(2 * utility::MPIUtils::get_num_local_ranks());
        std::iota(data_to_scatter.begin(), data_to_scatter.end(), 0);
        std::vector<int> offsets(utility::MPIUtils::get_num_local_ranks() + 1);
        for (auto i = 0; i <= utility::MPIUtils::get_num_local_ranks(); i++)
        {
            offsets[i] = 2 * i;
        }

        auto ret = utility::MPIUtils::scatter_arrays(data_to_scatter, offsets);

        assert(ret.size() == 2);
        assert(ret[0] == 2 * utility::MPIUtils::get_local_rank());
        assert(ret[1] == 2 * utility::MPIUtils::get_local_rank() + 1);
    }

    // Test MPIUtils::gather_strings_by_rank
    {
        std::string str  = "test";
        auto str_by_rank = utility::MPIUtils::gather_strings_by_rank(str);

        if (utility::MPIUtils::is_root())
        {
            assert(str_by_rank.size() == utility::MPIUtils::get_num_local_ranks());
            for (auto& v : str_by_rank)
            {
                assert(v == "test");
            }
        }
        else
        {
            assert(str_by_rank.empty());
        }
    }

    // Test MPIUtils::sum_of_lower_ranks
    {
        int32_t rank_id       = utility::MPIUtils::get_local_rank();
        auto value            = utility::MPIUtils::sum_of_lower_ranks(rank_id);
        int32_t correct_value = 0;
        for (auto i : utility::range(rank_id))
        {
            correct_value += i;
        }
        assert(value == correct_value);
    }
    {
        uint32_t rank_id       = utility::MPIUtils::get_local_rank();
        auto value             = utility::MPIUtils::sum_of_lower_ranks(rank_id);
        uint32_t correct_value = 0;
        for (auto i : utility::range(rank_id))
        {
            correct_value += i;
        }
        assert(value == correct_value);
    }
    {
        double rank_id       = utility::MPIUtils::get_local_rank();
        auto value           = utility::MPIUtils::sum_of_lower_ranks(rank_id);
        double correct_value = 0;
        for (auto i : utility::range(rank_id))
        {
            correct_value += i;
        }
        assert(value == correct_value);
    }

    // Test MPIUtils::gather_vectors_by_rank
    {
        std::vector<int> data_on_this_proc(4);
        std::iota(data_on_this_proc.begin(),
                  data_on_this_proc.end(),
                  utility::MPIUtils::get_local_rank());

        auto ret = utility::MPIUtils::gather_vectors_by_rank(data_on_this_proc);

        if (utility::MPIUtils::is_root())
        {
            assert(ret.size() == utility::MPIUtils::get_num_local_ranks());
            for (auto& v : ret)
            {
                assert(v.size() == 4);
                auto assert_val = utility::MPIUtils::get_local_rank();
                for (auto i : utility::indices(v))
                {
                    v[i] = assert_val++;
                }
            }
        }
        else
        {
            assert(ret.size() == 0);
        }
    }

    // Test find_exchange_destination_ranks
    {
        std::vector<int> source_ranks;
        if (!utility::MPIUtils::is_root())
        {
            source_ranks.push_back(utility::MPIUtils::get_local_rank() - 1);
        }
        auto dest_ranks = utility::MPIUtils::find_exchange_destination_ranks(source_ranks);
        if (utility::MPIUtils::get_local_rank() != utility::MPIUtils::get_num_local_ranks() - 1)
        {
            assert(dest_ranks.size() == 1);
            assert(dest_ranks[0] == utility::MPIUtils::get_local_rank() + 1);
        }
        else
        {
            assert(dest_ranks.size() == 0);
        }
    }

    utility::MPIUtils::finalize();

    return 0;
}
