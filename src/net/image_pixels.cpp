#include "image_pixels.h"

#include "utils/base64.h"

#include <png.h>

namespace banggame {

    uint32_t image_pixels::get_pixel(uint32_t x, uint32_t y) const {
        if (x >= width || y >= height) {
            return 0;
        }

        uint32_t result;
        uint32_t index = (y * width + x) * bytes_per_pixel;
        std::memcpy(&result, pixels.data() + index, bytes_per_pixel);
        return result;
    }

    void image_pixels::set_pixel(uint32_t x, uint32_t y, uint32_t value) {
        if (x < width && y < height) {
            uint32_t index = (y * width + x) * bytes_per_pixel;
            std::memcpy(pixels.data() + index, &value, bytes_per_pixel);
        }
    }
    
    image_pixels image_pixels::scale_to(uint32_t new_size) && {
        uint32_t new_width = width;
        uint32_t new_height = height;

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
            return std::move(*this);
        }

        image_pixels result { new_width, new_height };
        for (uint32_t y = 0; y < new_height; ++y) {
            for (uint32_t x = 0; x < new_width; ++x) {
                uint32_t scaled_x = x * width / new_width;
                uint32_t scaled_y = y * height / new_height;

                result.set_pixel(x, y, get_pixel(scaled_x, scaled_y));
            }
        }
        return result;
    }

    byte_vector image_to_png(image_pixels_view image) {
        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            throw std::runtime_error("Cannot create png write struct");
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_write_struct(&png, nullptr);
            throw std::runtime_error("Cannot create png info struct");
        }

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_write_struct(&png, &info);
            throw std::runtime_error("Error while writing png");
        }

        byte_vector result;

        // Set up custom write function to write
        png_set_write_fn(png, &result, [](png_structp png_ptr, png_bytep data, png_size_t length) {
            auto* vec = static_cast<byte_vector*>(png_get_io_ptr(png_ptr));
            vec->insert(vec->end(), data, data + length);
        }, nullptr);

        // Set image attributes
        png_set_IHDR(png, info, image.width, image.height, 8, PNG_COLOR_TYPE_RGBA,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_write_info(png, info);

        // Write image data row by row
        for (uint32_t y = 0; y < image.height; ++y) {
            png_write_row(png, static_cast<png_const_bytep>(image.pixels.data() + y * image.width * bytes_per_pixel));
        }

        png_write_end(png, nullptr);

        png_destroy_write_struct(&png, &info);

        return result;
    }

    image_pixels image_from_png_data_url(std::string_view data_url) {
        static constexpr std::string_view prefix = "data:image/png;base64,";
        if (!data_url.starts_with(prefix)) {
            throw std::runtime_error("Invalid data URL format");
        }
        byte_vector png_data = base64::base64_decode(data_url.substr(prefix.size()));

        // Initialize libpng structures
        png_image image{};
        image.version = PNG_IMAGE_VERSION;

        // Begin reading PNG
        if (!png_image_begin_read_from_memory(&image, png_data.data(), png_data.size())) {
            throw std::runtime_error("Failed to read PNG data from memory");
        }

        // Allocate memory for the image buffer
        image.format = PNG_FORMAT_RGBA;

        image_pixels result{ image.width, image.height };
        if (!png_image_finish_read(&image, nullptr, result.pixels.data(), 0, nullptr)) {
            png_image_free(&image);
            throw std::runtime_error("Failed to finish reading PNG image");
        }

        return result;
    }

}