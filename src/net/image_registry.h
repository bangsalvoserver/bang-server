#ifndef __IMAGE_REGISTRY_H__
#define __IMAGE_REGISTRY_H__

#include "image_pixels.h"
#include "utils/function_ref.h"

namespace banggame::image_registry {
    void register_image(banggame::image_pixels_view image);
    void deregister_image(banggame::image_pixels_view image);

    using write_png_function = tl::function_ref<void(std::string_view)>;
    bool write_image_png(size_t hash, const write_png_function &fun);

    template<std::convertible_to<banggame::image_pixels_view> T>
    class registered_image {
    private:
        T m_image;

    public:
        registered_image() = default;

        registered_image(const T &image): m_image{image} {
            register_image(m_image);
        }

        registered_image(T &&image): m_image{std::move(image)} {
            register_image(m_image);
        }

        registered_image(const registered_image &other): m_image{other.m_image} {
            register_image(m_image);
        }

        registered_image(registered_image &&other) noexcept : m_image{std::move(other.m_image)} {
            other.m_image = T{};
        }

        ~registered_image() {
            deregister_image(m_image);
        }

        registered_image &operator = (const registered_image &other) {
            if (this != &other) {
                deregister_image(m_image);
                m_image = other.m_image;
                register_image(m_image);
            }
            return *this;
        }

        registered_image &operator = (registered_image &&other) noexcept {
            if (this != &other) {
                deregister_image(m_image);
                m_image = std::move(other.m_image);
                other.m_image = T{};
            }
            return *this;
        }

        operator banggame::image_pixels_view() const {
            return static_cast<banggame::image_pixels_view>(m_image);
        }

        explicit operator bool() const {
            return static_cast<bool>(m_image);
        }
    };

    using image_pixels_view = registered_image<banggame::image_pixels_view>;
    using image_pixels = registered_image<banggame::image_pixels>;
}

#endif