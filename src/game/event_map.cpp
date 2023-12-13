#include "event_map.h"

namespace banggame {
    
    void listener_map::do_add_listener(event_card_key key, listener_ptr &&ptr) {
        auto it = m_listeners.emplace(std::move(ptr));
        m_map.emplace(key, it);
    }

    void listener_map::do_remove_listeners(iterator_map_range range) {
        if (range.empty()) return;

        for (const auto &[key, it] : range) {
            auto &listener = **it;
            if (listener.active) {
                if (m_lock) {
                    m_to_remove.push_back(it);
                    listener.active = false;
                } else {
                    m_listeners.erase(it);
                }
            }
        }
        m_map.erase(range.begin(), range.end());
    }

    void listener_map::do_call_event(size_t id, detail::event_listener_invoke_fun fun, const void *tuple) {
        auto [low, high] = m_listeners.equal_range(id);
        if (low == high) return;

        ++m_lock;
        for (auto it = low; it != high; ++it) {
            const auto &listener = **it;
            if (listener.id == id && listener.active) {
                std::invoke(fun, listener, tuple);
            }
        }
        --m_lock;
        
        if (!m_lock && !m_to_remove.empty()) {
            for (auto it : m_to_remove) {
                m_listeners.erase(it);
            }
            m_to_remove.clear();
        }
    }
}