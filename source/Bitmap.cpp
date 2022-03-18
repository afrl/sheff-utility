#include "utility/Bitmap.h"

#include "utility/serialization/Serialization.h"

namespace utility
{

namespace detail
{
// Based on blog post: https://dev.to/muiz6/c-how-to-write-a-bitmap-image-from-scratch-1k6m

struct BitmapHeader
{
    // File header information
    std::array<char, 2> bitmap_signature;
    uint32_t file_size_bytes;
    uint32_t reserved_bytes;
    uint32_t pixel_data_offset;

    // Data header information
    uint32_t data_header_size;
    int32_t n_pixel_width;
    int32_t n_pixel_height;
    uint16_t n_color_planes;
    uint16_t color_depth;
    uint32_t compression_method;
    uint32_t raw_bmp_data_size; // Only > 0 for compressed images
    int32_t n_pixel_per_meter_horizontal;
    int32_t n_pixel_per_meter_vertical;
    uint32_t color_table_entries;
    uint32_t important_colors;

    /// serialize(...) specialization
    template<typename SerializerType>
    void serialize(SerializerType& s) const
    {
        s(named_value("bitmap_signature", Slice<const char>(bitmap_signature.data(), bitmap_signature.size())));
        s(named_value("file_size_bytes", file_size_bytes));
        s(named_value("reserved_bytes", reserved_bytes));
        s(named_value("pixel_data_offset", pixel_data_offset));

        s(named_value("data_header_size", data_header_size));
        s(named_value("n_pixel_width", n_pixel_width));
        s(named_value("n_pixel_height", n_pixel_height));
        s(named_value("n_color_planes", n_color_planes));
        s(named_value("color_depth", color_depth));
        s(named_value("compression_method", compression_method));
        s(named_value("raw_bmp_data_size", raw_bmp_data_size));
        s(named_value("n_pixel_per_meter_horizontal", n_pixel_per_meter_horizontal));
        s(named_value("n_pixel_per_meter_vertical", n_pixel_per_meter_vertical));
        s(named_value("color_table_entries", color_table_entries));
        s(named_value("important_colors", important_colors));
    }
};

/// @brief Creates a simple bit map header that must go at the beginning of a bit map file.
BitmapHeader create_header(const int32_t width,
                           const int32_t height,
                           const Slice<uint8_t>& aligned_pixel_data)
{
    uint32_t n_pixel_bytes = static_cast<uint32_t>(aligned_pixel_data.size());

    BitmapHeader header = { { 'B', 'M' },            // bitmap_signature
                            14 + 40 + n_pixel_bytes, // file_size_bytes
                            0,                       // reserved_bytes
                            54,                      // pixel_data_offset
                            40,                      // data_header_size
                            width,                   // n_pixel_width
                            -height, // n_pixel_height (sign indicates start at top-left pixel)
                            1,       // n_color_planes
                            24,      // color_depth
                            0,       // compression_method
                            0,       // raw_bmp_data_size
                            100,     // n_pixel_per_meter_horizontal
                            100,     // n_pixel_per_meter_vertical
                            0,       // color_table_entries
                            0 };     // important_colors
    return header;
}

/// @brief Converts grayscale pixel array to color
std::vector<std::array<uint8_t, 3>> grayscale_to_color(const Slice<uint8_t>& pixel_data)
{
    std::vector<std::array<uint8_t, 3>> color_data(pixel_data.size());
    for (auto i : indices(pixel_data))
    {
        for (auto& v : color_data[i])
        {
            v = pixel_data[i];
        }
    }
    return color_data;
}

/// @brief Given an array of (blue, green, red) pixel data, this returns a vector of the same data
///        that is 4-byte aligned.
std::vector<uint8_t> to_4_byte_aligned(const int32_t width,
                                       const int32_t height,
                                       const Slice<const uint8_t>& bgr_pixel_data)
{
    const auto n_bytes_per_pixel = 3;
    assert(n_bytes_per_pixel * width * height == bgr_pixel_data.size());

    auto pad_size = 4 - (n_bytes_per_pixel * width) % 4;
    if (pad_size == 4)
    {
        return std::vector<uint8_t>(bgr_pixel_data.begin(), bgr_pixel_data.end());
    }

    std::vector<uint8_t> aligned_data(bgr_pixel_data.size() + height * pad_size, 0);
    for (auto row : range(height))
    {
        const auto row_start         = n_bytes_per_pixel * width * row;
        const auto row_end           = n_bytes_per_pixel * width * (row + 1);
        const auto aligned_row_start = (n_bytes_per_pixel * width + pad_size) * row;
        std::copy(bgr_pixel_data.begin() + row_start,
                  bgr_pixel_data.begin() + row_end,
                  aligned_data.begin() + aligned_row_start);
    }
    return aligned_data;
}


int64_t print_bitmap_data(const int32_t width,
                          const int32_t height,
                          const Slice<std::array<uint8_t, 3>> pixel_data)
{
    // Note: the bit map requires that rows are padded such that the data is aligned by 4 bytes
    auto aligned_pixel_data =
      to_4_byte_aligned(width,
                        height,
                        Slice<const uint8_t>(pixel_data[0].data(), 3 * pixel_data.size()));
    auto header = detail::create_header(width, height, slice(aligned_pixel_data));

    JSONSerializer s(std::cout);
    s(named_value("header", header));
    s(named_value("aligned_pixel_data",
                    Slice<const uint8_t>(aligned_pixel_data.data(), aligned_pixel_data.size())));

    SerialByteCounter counter;
    counter(header);
    counter(Slice<const uint8_t>(aligned_pixel_data.data(), aligned_pixel_data.size()));
    return counter.n_bytes();
}

int64_t print_bitmap_data(const int32_t width,
                          const int32_t height,
                          const Slice<uint8_t> pixel_data)
{
    auto color_data = grayscale_to_color(pixel_data);
    return print_bitmap_data(width, height, slice(color_data));
}

} // end namespace detail

void save_bitmap(const std::string& file_path,
                 const int32_t width,
                 const int32_t height,
                 const Slice<std::array<uint8_t, 3>> pixel_data)
{
    // Note: adding the binary option is very important.  Otherwise, pixel data that happens
    // to be 10 (which is the char '\n') is implicitly converted to 13 10 ('\r\n')
    std::ofstream out_file(file_path.c_str(), std::ios_base::binary);
    OutputSerializer s(out_file);

    // Note: the bit map requires that rows are padded such that the data is aligned by 4 bytes
    auto aligned_pixel_data =
      detail::to_4_byte_aligned(width,
                                height,
                                Slice<const uint8_t>(pixel_data[0].data(), 3 * pixel_data.size()));

    auto header = detail::create_header(width, height, slice(aligned_pixel_data));

    s(header);
    s(Slice<const uint8_t>(aligned_pixel_data.data(), aligned_pixel_data.size()));

    out_file.close();
}

void save_bitmap(const std::string& file_path,
                 const int32_t width,
                 const int32_t height,
                 const Slice<uint8_t> pixel_data)
{
    auto color_data = detail::grayscale_to_color(pixel_data);
    save_bitmap(file_path, width, height, slice(color_data));
}

}