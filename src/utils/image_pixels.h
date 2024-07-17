#ifndef __IMAGE_PIXELS_H__
#define __IMAGE_PIXELS_H__

#include "utils/base64.h"
#include "utils/json_aggregate.h"

namespace utils {
    struct image_pixels {
        int width;
        int height;
        std::vector<std::byte> pixels;
    };
}

namespace json {

    struct image_pixels_tag {};

    template<>
    struct serializer<std::vector<std::byte>, image_pixels_tag> {
        serializer(image_pixels_tag) {}

        json operator()(const std::vector<std::byte> &value) const {
            return base64::base64_encode(value);
        }
    };

    template<typename Context>
    struct serializer<utils::image_pixels, Context>
        : aggregate_serializer_unchecked<utils::image_pixels, image_pixels_tag>
    {
        serializer() : aggregate_serializer_unchecked(image_pixels_tag{}) {}
    };

    template<>
    struct deserializer<std::vector<std::byte>, image_pixels_tag> {
        deserializer(image_pixels_tag) {}
        
        std::vector<std::byte> operator()(const json &value) const {
            if (!value.is_string()) {
                throw std::runtime_error("Cannot deserialize base64 encoded string");
            }
            return base64::base64_decode(value.get<std::string>());
        }
    };

    template<typename Context>
    struct deserializer<utils::image_pixels, Context> {
        utils::image_pixels operator()(const json &value) const {
            aggregate_deserializer_unchecked<utils::image_pixels, image_pixels_tag> base{image_pixels_tag{}};
            utils::image_pixels result = base(value);
            if (result.pixels.size() != result.width * result.height * 4) {
                throw std::runtime_error("Invalid image");
            }
            return result;
        }
    };
}

#endif