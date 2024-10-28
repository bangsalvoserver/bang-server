#ifndef __IMAGE_REGISTRY_H__
#define __IMAGE_REGISTRY_H__

#include "image_pixels.h"
#include "utils/function_ref.h"

namespace banggame::image_registry {
    void register_image(image_pixels_hash hash, image_pixels_view image);
    void register_image(image_pixels_hash hash);
    void deregister_image(image_pixels_hash hash);

    using write_png_function = tl::function_ref<void(byte_slice)>;
    bool write_image_png(image_pixels_hash hash, const write_png_function &fun);

    class registered_image {
    private:
        image_pixels_hash m_hash;

    public:
        registered_image() = default;

        registered_image(image_pixels_view image): m_hash{image} {
            register_image(m_hash, image);
        }

        registered_image(const registered_image &other): m_hash{other.m_hash} {
            register_image(m_hash);
        }

        registered_image(registered_image &&other) noexcept : m_hash{other.m_hash} {
            other.m_hash = {};
        }

        ~registered_image() {
            deregister_image(m_hash);
        }

        registered_image &operator = (const registered_image &other) {
            if (m_hash != other.m_hash) {
                deregister_image(m_hash);
                m_hash = other.m_hash;
                register_image(m_hash);
            }
            return *this;
        }

        registered_image &operator = (registered_image &&other) noexcept {
            if (m_hash != other.m_hash) {
                deregister_image(m_hash);
                m_hash = other.m_hash;
                other.m_hash = {};
            }
            return *this;
        }

        void reset(image_pixels_view image) {
            image_pixels_hash hash{image};
            if (m_hash != hash) {
                deregister_image(m_hash);
                m_hash = hash;
                register_image(m_hash, image);
            }
        }

        operator image_pixels_hash() const {
            return m_hash;
        }
    };
}

#endif