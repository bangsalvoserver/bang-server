#ifndef __IMAGE_PIXELS_H__
#define __IMAGE_PIXELS_H__

#include "utils/json_serial.h"

namespace banggame {

    using byte_slice = std::span<const uint8_t>;
    using byte_vector = std::vector<uint8_t>;

    static constexpr int bytes_per_pixel = 4;

    struct image_pixels_view {
        uint32_t width;
        uint32_t height;
        byte_slice pixels;
    };

    struct image_pixels_hash {
        size_t value = 0;

        constexpr image_pixels_hash(size_t value): value{value} {}

        constexpr image_pixels_hash(image_pixels_view image = {}) {
            value ^= static_cast<size_t>(image.width) + 0x9e3779b9 + (value << 6) + (value >> 2);
            value ^= static_cast<size_t>(image.height) + 0x9e3779b9 + (value << 6) + (value >> 2);

            for (uint8_t byte : image.pixels) {
                value ^= static_cast<size_t>(byte) + 0x9e3779b9 + (value << 6) + (value >> 2);
            }
        }

        explicit constexpr operator bool() const {
            static constexpr image_pixels_view empty_hash{}; 
            return *this != empty_hash;
        }

        constexpr bool operator == (const image_pixels_hash &other) const = default;
    };

    class image_pixels {
    private:
        uint32_t width;
        uint32_t height;
        byte_vector pixels;

    public:
        image_pixels() = default;
        image_pixels(uint32_t width, uint32_t height)
            : width{width}, height{height}
        {
            pixels.resize(width * height * bytes_per_pixel);
        }

        operator image_pixels_view() const {
            return { width, height, pixels };
        }

        uint32_t get_pixel(uint32_t x, uint32_t y) const;
        void set_pixel(uint32_t x, uint32_t y, uint32_t value);

        image_pixels scale_to(uint32_t new_size) &&;

        friend image_pixels image_from_png_data_url(std::string_view data_url);
    };

    byte_vector image_to_png(image_pixels_view image);

    image_pixels image_from_png_data_url(std::string_view data_url);
}

namespace json {

    template<typename Context>
    struct serializer<banggame::image_pixels_hash, Context> {
        json operator()(banggame::image_pixels_hash value) const {
            if (value) {
                return std::format("{:x}", value.value);
            }
            return {};
        }
    };

    template<typename Context>
    struct deserializer<banggame::image_pixels, Context> {
        banggame::image_pixels operator()(const json &value) const {
            if (value.is_null()) {
                return {};
            }
            if (!value.is_string()) {
                throw deserialize_error("Cannot deserialize image_pixels");
            }
            return banggame::image_from_png_data_url(value.get<std::string_view>());
        }
    };
}

#endif