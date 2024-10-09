#include "event_map.h"

#include "net/logging.h"
#include "utils/type_name.h"

namespace std {
    template<> struct formatter<banggame::event_listener_key> {
        constexpr auto parse(std::format_parse_context &ctx) {
            return ctx.begin();
        }

        auto format(const banggame::event_listener_key &value, std::format_context &ctx) const {
            return std::format_to(ctx.out(), "{}: [{}]", value.key, utils::demangle(value.type.name()));
        }
    };

    template<> struct formatter<banggame::event_listener> : formatter<std::string_view> {
        auto format(const banggame::event_listener &value, std::format_context &ctx) const {
            return formatter<std::string_view>::format(utils::demangle(value.target_type().name()), ctx);
        }
    };
}

namespace banggame {
    
    void listener_map::do_add_listener(event_listener_key key, event_listener &&listener) {
        auto it = m_listeners.emplace(key, std::move(listener));
        logging::debug("add_listener() on {} {}", it->first, it->second);
        m_map.emplace(key.key, it);
    }

    void listener_map::do_remove_listeners(iterator_map_range range) {
        if (range.empty()) return;

        for (auto it : range | rv::values) {
            auto &[key, listener] = *it;
            if (listener.is_active()) {
                logging::debug("remove_listener() on {} {}", key, listener);
                if (m_lock) {
                    m_to_remove.push_back(it);
                    listener.deactivate();
                } else {
                    m_listeners.erase(it);
                }
            }
        }
        m_map.erase(range.begin(), range.end());
    }

    void listener_map::do_call_event(std::type_index type, const void *tuple) {
        auto [low, high] = m_listeners.equal_range(type);
        rn::subrange range(low, high);
        if (range.empty()) return;

        ++m_lock;
        for (auto &[key, listener] : range) {
            if (key.type == type && listener.is_active()) {
                logging::trace("call_event() on {} {}", key, listener);
                std::invoke(listener, tuple);
            }
        }
        --m_lock;
        
        if (!m_lock && !m_to_remove.empty()) {
            for (listener_iterator listener : m_to_remove) {
                m_listeners.erase(listener);
            }
            m_to_remove.clear();
        }
    }
}