#include "image_pixels.h"

#include <mutex>
#include <unordered_set>

namespace banggame::image_registry {

    struct image_pixels_hasher {
        using is_transparent = void;

        size_t operator()(const banggame::image_pixels_view &image) const {
            return image.get_hash();
        }

        size_t operator()(size_t hash) const {
            return hash;
        }
    };

    struct image_pixels_equal {
        using is_transparent = void;

        bool operator()(const banggame::image_pixels_view &lhs, const banggame::image_pixels_view &rhs) const {
            return lhs.width == rhs.width
                && lhs.height == rhs.height
                && lhs.pixels.data() == rhs.pixels.data();
        }
        
        bool operator()(const banggame::image_pixels_view &lhs, size_t hash) const {
            return lhs.get_hash() == hash;
        }
        
        bool operator()(size_t hash, const banggame::image_pixels_view &rhs) const {
            return rhs.get_hash() == hash;
        }
    };

    struct registry {
    private:
        std::unordered_multiset<banggame::image_pixels_view, image_pixels_hasher, image_pixels_equal> m_registry;
        mutable std::mutex m_mutex;

        registry() = default;

    public:
        static registry &get() {
            static registry instance;
            return instance;
        }

        void register_image(banggame::image_pixels_view image) {
            if (image) {
                std::scoped_lock guard{m_mutex};
                m_registry.emplace(image);
            }
        }

        void deregister_image(banggame::image_pixels_view image) {
            if (image) {
                std::scoped_lock guard{m_mutex};
                m_registry.erase(image);
            }
        }

        banggame::image_pixels_view get_image(size_t hash) const {
            std::scoped_lock guard{m_mutex};
            auto it = m_registry.find(hash);
            if (it == m_registry.end()) {
                return {};
            }
            return *it;
        }
    };
    
    void register_image(banggame::image_pixels_view image) {
        registry::get().register_image(image);
    }

    void deregister_image(banggame::image_pixels_view image) {
        registry::get().deregister_image(image);
    }

    banggame::image_pixels_view get_image(size_t hash) {
        return registry::get().get_image(hash);
    }
}