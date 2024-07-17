#ifndef __IMAGE_PIXELS_H__
#define __IMAGE_PIXELS_H__

#include "utils/base64.h"
#include "utils/json_serial.h"

namespace utils {
    struct image_pixels {
        int width;
        int height;
        std::vector<std::byte> pixels;
    };
}

namespace json {
    template<typename Context>
    struct serializer<utils::image_pixels, Context> {
        json operator()(const utils::image_pixels &value) const {
            return {
                {"width", value.width},
                {"height", value.height},
                {"pixels", base64::base64_encode(value.pixels)}
            };
        }
    };

    template<typename Context>
    struct deserializer<utils::image_pixels, Context> {
        std::vector<std::byte> deserialize_base64_string(const json &value) const {
            if (!value.is_string()) {
                throw std::runtime_error("Cannot deserialize base64 encoded string");
            }
            return base64::base64_decode(value.get<std::string>());
        }

        utils::image_pixels operator()(const json &value) const {
            if (!value.is_object()) {
                throw std::runtime_error("Cannot deserialize image_pixels: value is not object");
            }
            utils::image_pixels result {
                value.contains("width") ? deserializer<int>{}(value["width"]) : 0,
                value.contains("height") ? deserializer<int>{}(value["height"]) : 0,
                value.contains("pixels") ? deserialize_base64_string(value["pixels"]) : std::vector<std::byte>{}
            };
            if (result.pixels.size() != result.width * result.height * 4) {
                throw std::runtime_error("Invalid image");
            }
            return result;
        }
    };
}

#endif