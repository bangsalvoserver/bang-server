#ifndef __IMAGE_PIXELS_H__
#define __IMAGE_PIXELS_H__

#include "utils/base64.h"
#include "utils/json_aggregate.h"

#include "compression.h"

namespace utils {
    struct image_pixels {
        int width;
        int height;
        std::vector<std::byte> pixels;

        explicit operator bool () const {
            return width != 0 && height != 0;
        }

        uint32_t get_pixel(size_t x, size_t y) const {
            if (x >= width || y >= height) {
                return 0;
            }

            uint32_t result;
            size_t index = (y * width + x) * 4;
            std::memcpy(&result, pixels.data() + index, 4);
            return result;
        }

        void set_pixel(size_t x, size_t y, uint32_t value) {
            if (x < width && y < height) {
                size_t index = (y * width + x) * 4;
                std::memcpy(pixels.data() + index, &value, 4);
            }
        }
    };
}

namespace json {

    struct image_pixels_tag {};

    template<> struct serializer<std::vector<std::byte>, image_pixels_tag> {
        json operator()(const std::vector<std::byte> &value) const {
            return base64::base64_encode(compression::compress_bytes(value));
        }
    };

    template<typename Context>
    struct serializer<utils::image_pixels, Context> {
        using base_type = aggregate_serializer_unchecked<utils::image_pixels, image_pixels_tag>;
        json operator()(const utils::image_pixels &value) const {
            return base_type{}(value, image_pixels_tag{});
        }
    };

    template<> struct deserializer<std::vector<std::byte>, image_pixels_tag> {
        std::vector<std::byte> operator()(const json &value) const {
            if (!value.is_string()) {
                throw deserialize_error("Cannot deserialize base64 encoded string");
            }
            return base64::base64_decode(value.get<std::string>());
        }
    };

    template<typename Context>
    struct deserializer<utils::image_pixels, Context> {
        using base_type = aggregate_deserializer_unchecked<utils::image_pixels, image_pixels_tag>;
        utils::image_pixels operator()(const json &value) const {
            if (value.is_null()) {
                return {};
            }
            utils::image_pixels result = base_type{}(value, image_pixels_tag{});
            result.pixels = compression::decompress_bytes(result.pixels, result.width * result.height * 4);
            return result;
        }
    };
}

#endif