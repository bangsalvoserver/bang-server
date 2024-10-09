#ifndef __IMAGE_PIXELS_H__
#define __IMAGE_PIXELS_H__

#include "utils/base64.h"
#include "utils/json_aggregate.h"

#include "compression.h"

namespace banggame {
    struct image_pixels {
        int width;
        int height;
        std::vector<uint8_t> pixels;

        explicit operator bool () const {
            return width != 0 && height != 0;
        }

        uint32_t get_pixel(size_t x, size_t y) const;
        void set_pixel(size_t x, size_t y, uint32_t value);

        image_pixels scale_to(int new_size) const;
    };
}

namespace json {

    struct image_pixels_tag {};

    template<> struct serializer<std::vector<uint8_t>, image_pixels_tag> {
        json operator()(const std::vector<uint8_t> &value) const {
            return base64::base64_encode(compression::compress_bytes(value));
        }
    };

    template<typename Context>
    struct serializer<banggame::image_pixels, Context> {
        using base_type = aggregate_serializer_unchecked<banggame::image_pixels, image_pixels_tag>;
        json operator()(const banggame::image_pixels &value) const {
            return base_type{}(value, image_pixels_tag{});
        }
    };

    template<> struct deserializer<std::vector<uint8_t>, image_pixels_tag> {
        std::vector<uint8_t> operator()(const json &value) const {
            if (!value.is_string()) {
                throw deserialize_error("Cannot deserialize base64 encoded string");
            }
            return compression::decompress_bytes(base64::base64_decode(value.get<std::string>()));
        }
    };

    template<typename Context>
    struct deserializer<banggame::image_pixels, Context> {
        using base_type = aggregate_deserializer_unchecked<banggame::image_pixels, image_pixels_tag>;
        banggame::image_pixels operator()(const json &value) const {
            if (value.is_null()) {
                return {};
            }
            banggame::image_pixels result = base_type{}(value, image_pixels_tag{});
            if (result.pixels.size() != result.width * result.height * 4) {
                throw deserialize_error("Invalid image");
            }
            return result;
        }
    };
}

#endif