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

        auto ret = utility::MPIUtils::scatter_arrays(
            data_to_scatter,
            offsets);

        assert(ret.size() == 2);
        assert(ret[0] == 2 * utility::MPIUtils::get_local_rank());
        assert(ret[1] == 2 * utility::MPIUtils::get_local_rank() + 1);
    }

    {
        std::string str = "test";
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

    return 0; 
}
