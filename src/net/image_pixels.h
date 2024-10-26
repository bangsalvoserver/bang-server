#ifndef __IMAGE_PIXELS_H__
#define __IMAGE_PIXELS_H__

#include "utils/json_serial.h"

namespace banggame {

    struct image_pixels_view {
        int width;
        int height;
        std::span<const uint8_t> pixels;

        explicit operator bool () const {
            return width != 0 && height != 0 && !pixels.empty();
        }
        
        size_t get_hash() const;
    };

    struct image_pixels {
        int width;
        int height;
        std::vector<uint8_t> pixels;

        explicit operator bool () const {
            return width != 0 && height != 0 && !pixels.empty();
        }

        operator image_pixels_view() const {
            return { width, height, pixels };
        }

        uint32_t get_pixel(size_t x, size_t y) const;
        void set_pixel(size_t x, size_t y, uint32_t value);

        image_pixels scale_to(int new_size) const;
    };

    std::string image_to_png(image_pixels_view image);

    image_pixels image_from_png_data_url(std::string_view data_url);
}

namespace json {

    template<typename Context>
    struct serializer<banggame::image_pixels_view, Context> {
        json operator()(const banggame::image_pixels_view &value) const {
            if (value) {
                return std::format("{:x}", value.get_hash());
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