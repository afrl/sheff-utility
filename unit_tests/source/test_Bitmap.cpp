/// Unit test for Bitset.h

#include "utility/Bitmap.h"

#include <assert.h>
#include <fstream>
#include <vector>

int main()
{
    auto n_bytes = utility::detail::print_bitmap_data(0, 0, utility::Slice<uint8_t>());
    assert(n_bytes == 54);

    std::vector<uint8_t> pixel_data = { 0, 100, 200, 255 };

    n_bytes = utility::detail::print_bitmap_data(2, 2, utility::slice(pixel_data));
    assert(n_bytes == 54 + 12 + 4 /* padding */);

    utility::save_bitmap("grayscale.bmp", 2, 2, utility::slice(pixel_data));

    return 0;
}
