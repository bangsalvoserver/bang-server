#ifndef __IMAGE_REGISTRY_H__
#define __IMAGE_REGISTRY_H__

#include "image_pixels.h"

#include <shared_mutex>

namespace banggame::image_registry {
    void register_image(image_pixels_hash hash, image_pixels_view image);
    void register_image(image_pixels_hash hash);
    void deregister_image(image_pixels_hash hash);

    using lock_bytes = std::pair<std::shared_lock<std::shared_mutex>, byte_slice>;
    lock_bytes get_png_image_data(image_pixels_hash hash);

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