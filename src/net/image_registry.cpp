#include "image_registry.h"

#include <unordered_map>
#include <mutex>

#include "logging.h"

namespace std {
    template<> struct hash<banggame::image_pixels_hash> {
        size_t operator()(const banggame::image_pixels_hash &value) const {
            return value.value;
        }
    };
}

namespace banggame::image_registry {

    struct registry_entry {
        byte_vector png_bytes;
        size_t refcount = 0;

        registry_entry(image_pixels_view image)
            : png_bytes{image_to_png(image)} {}
    };

    class registry {
    private:
        std::unordered_map<image_pixels_hash, registry_entry> m_registry;
        mutable std::shared_mutex m_mutex;

        registry() = default;

    public:
        static registry &get() {
            static registry instance;
            return instance;
        }

        void register_image(image_pixels_hash hash, image_pixels_view image) {
            if (hash) {
                try {
                    std::scoped_lock guard{m_mutex};
                    auto [it, inserted] = m_registry.try_emplace(hash, image);
                    ++it->second.refcount;
                } catch (const std::exception &e) {
                    logging::error("Error while registering image: {}", e.what());
                }
            }
        }

        void register_image(image_pixels_hash hash) {
            if (hash) {
                std::scoped_lock guard{m_mutex};
                auto it = m_registry.find(hash);
                if (it != m_registry.end()) {
                    ++it->second.refcount;
                }
            }
        }

        void deregister_image(image_pixels_hash hash) {
            if (hash) {
                std::scoped_lock guard{m_mutex};
                auto it = m_registry.find(hash);
                if (it != m_registry.end() && --it->second.refcount == 0) {
                    m_registry.erase(it);
                }
            }
        }

        lock_bytes get_png_image_data(image_pixels_hash hash) const {
            std::shared_lock guard{m_mutex};
            auto it = m_registry.find(hash);
            if (it == m_registry.end()) {
                return {};
            }
            return { std::move(guard), it->second.png_bytes };
        }
    };
    
    void register_image(image_pixels_hash hash, image_pixels_view image) {
        registry::get().register_image(hash, image);
    }
    
    void register_image(image_pixels_hash hash) {
        registry::get().register_image(hash);
    }

    void deregister_image(image_pixels_hash hash) {
        registry::get().deregister_image(hash);
    }

    lock_bytes get_png_image_data(image_pixels_hash hash) {
        return registry::get().get_png_image_data(hash);
    }
}