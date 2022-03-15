#pragma once

#include "utility/Slice.h"

#include <array>
#include <string>
#include <vector>

namespace utility
{
namespace detail
{
/// @brief Used to debug the color version of save_bitmap
/// @returns The the binary size of the data if it was serialized
int64_t print_bitmap_data(const int32_t width,
                          const int32_t height,
                          const Slice<std::array<uint8_t, 3>> pixel_data);

/// @brief Used to debug the grayscale version of save_bitmap
/// @returns The the binary size of the data if it was serialized
int64_t print_bitmap_data(const int32_t width,
                          const int32_t height,
                          const Slice<uint8_t> pixel_data);
}

/// @brief Saves color pixel data to a bit map image file.
///
/// @param file_path - full path or relative path (to current working directory) with extension
/// @param width - # of pixels along the width of the image
/// @param height - # of pixels along the height of the image
/// @param pixel_data - color for each pixel starting at the top-left of the image, with each pixel
///        having a triplet between 0 and 255 indicating the blue, green, and red respectively.
void save_bitmap(const std::string& file_path,
                 const int32_t width,
                 const int32_t height,
                 const Slice<std::array<uint8_t, 3>> pixel_data);

/// @brief Saves grayscale pixel data to a bit map image file.
///
/// @param file_path - full path or relative path (to current working directory) with extension
/// @param width - # of pixels along the width of the image
/// @param height - # of pixels along the height of the image
/// @param pixel_data - color for each pixel starting at the top-left of the image, with each pixel
///        having a triplet between 0 and 255 indicating the blue, green, and red respectively.
void save_bitmap(const std::string& file_path,
                 const int32_t width,
                 const int32_t height,
                 const Slice<uint8_t> pixel_data);
}
