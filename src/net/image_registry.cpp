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

    static std::unordered_multiset<banggame::image_pixels_view, image_pixels_hasher, image_pixels_equal> m_registry;
    static std::mutex m_registry_mutex;
    
    void register_image(banggame::image_pixels_view image) {
        if (image) {
            std::scoped_lock guard{m_registry_mutex};
            m_registry.emplace(image);
        }
    }

    void deregister_image(banggame::image_pixels_view image) {
        if (image) {
            std::scoped_lock guard{m_registry_mutex};
            m_registry.erase(image);
        }
    }

    std::optional<banggame::image_pixels_view> get_image(size_t hash) {
        std::scoped_lock guard{m_registry_mutex};
        
        auto it = m_registry.find(hash);
        if (it == m_registry.end()) {
            return std::nullopt;
        }
        return *it;
    }
}