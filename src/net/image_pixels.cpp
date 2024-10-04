#include "image_pixels.h"

namespace banggame {

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

        if (new_width == width && new_height == height) {
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
}