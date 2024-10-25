#include "image_pixels.h"

#include "utils/base64.h"

#include <png.h>

namespace banggame {

    size_t image_pixels_view::get_hash() const {
        size_t seed = 0;

        seed ^= std::hash<int>{}(width) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(height) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<const uint8_t *>{}(pixels.data()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        return seed;
    }

    uint32_t image_pixels::get_pixel(size_t x, size_t y) const {
        if (x >= width || y >= height) {
            return 0;
        }

        uint32_t result;
        size_t index = (y * width + x) * 4;
        std::memcpy(&result, pixels.data() + index, 4);
        return result;
    }

    void image_pixels::set_pixel(size_t x, size_t y, uint32_t value) {
        if (x < width && y < height) {
            size_t index = (y * width + x) * 4;
            std::memcpy(pixels.data() + index, &value, 4);
        }
    }
    
    image_pixels image_pixels::scale_to(int new_size) const {
        int new_width = width;
        int new_height = height;

        if (new_width <= 0 || new_height <= 0 || new_size <= 0) {
            return image_pixels{};
        }

        if (new_width > new_height) {
            new_height = new_size * new_height / new_width;
            new_width = new_size;
        } else {
            new_width = new_size * new_width / new_height;
            new_height = new_size;
        }

        if (new_width >= width && new_height >= height) {
            return *this;
        }

        image_pixels result { new_width, new_height };
        result.pixels.resize(new_width * new_height * 4);
        for (size_t y = 0; y < new_height; ++y) {
            for (size_t x = 0; x < new_width; ++x) {
                size_t scaled_x = x * width / new_width;
                size_t scaled_y = y * height / new_height;

                result.set_pixel(x, y, get_pixel(scaled_x, scaled_y));
            }
        }
        return result;
    }

    bool image_pixels_view::write_png(write_bytes_fun write_fn) const {
        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            return false;
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_write_struct(&png, nullptr);
            return false;
        }

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_write_struct(&png, &info);
            return false;
        }

        // Set up custom write function to write
        png_set_write_fn(png, &write_fn, [](png_structp png_ptr, png_bytep data, png_size_t length) {
            auto* fun = static_cast<write_bytes_fun*>(png_get_io_ptr(png_ptr));
            (*fun)(std::string_view{
                reinterpret_cast<const char *>(data),
                static_cast<size_t>(length)
            });
        }, nullptr);

        // Set image attributes
        int color_type = PNG_COLOR_TYPE_RGBA;
        png_set_IHDR(png, info, width, height, 8, color_type,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_write_info(png, info);

        // Write image data row by row
        const int bytes_per_pixel = 4; // RGBA has 4 bytes per pixel
        std::vector<png_bytep> row_pointers(height);

        for (int y = 0; y < height; ++y) {
            row_pointers[y] = (png_bytep)(pixels.data() + y * width * bytes_per_pixel);
        }

        png_write_image(png, row_pointers.data());
        png_write_end(png, nullptr);

        png_destroy_write_struct(&png, &info);

        return true;
    }

    image_pixels image_pixels::from_png_data_url(std::string_view data_url) {
        static constexpr std::string_view prefix = "data:image/png;base64,";
        if (!data_url.starts_with(prefix)) {
            throw std::runtime_error("Invalid data URL format");
        }
        std::vector<uint8_t> png_data = base64::base64_decode(data_url.substr(prefix.size()));

        // Initialize libpng structures
        png_image image{};
        image.version = PNG_IMAGE_VERSION;

        // Begin reading PNG
        if (!png_image_begin_read_from_memory(&image, png_data.data(), png_data.size())) {
            throw std::runtime_error("Failed to read PNG data from memory");
        }

        // Allocate memory for the image buffer
        image.format = PNG_FORMAT_RGBA;
        std::vector<uint8_t> pixels;
        pixels.resize(PNG_IMAGE_SIZE(image));
        if (!png_image_finish_read(&image, nullptr, pixels.data(), 0, nullptr)) {
            png_image_free(&image);
            throw std::runtime_error("Failed to finish reading PNG image");
        }

        return {
            static_cast<int>(image.width),
            static_cast<int>(image.height),
            std::move(pixels)
        };
    }

}