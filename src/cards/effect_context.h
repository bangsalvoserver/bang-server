#ifndef __EFFECT_CONTEXT_H__
#define __EFFECT_CONTEXT_H__

#include <any>
#include <typeindex>
#include <unordered_map>
#include <functional>

#include "card_serial.h"

namespace banggame {

    template<typename T>
    concept aggregate_with_one_member = requires {
        requires std::is_aggregate_v<T>;
        requires (reflect::size<T>() == 1);
    };

    template<typename T>
    decltype(auto) unwrap_value(T &&value) {
        if constexpr (aggregate_with_one_member<std::remove_cvref_t<T>>) {
            auto &&[unwrapped] = std::forward<T>(value);
            return (unwrapped);
        } else {
            return value;
        }
    }

    template<typename T>
    using unwrapped_t = std::remove_cvref_t<decltype(unwrap_value(std::declval<T>()))>;

    class context_entry {
    private:
        using serialize_fun_t = void (*)(const context_entry &self, json::string_writer &writer);

        std::any value;
        serialize_fun_t serialize_fun;

        template<typename T>
        static constexpr serialize_fun_t make_serialize_fun() {
            if constexpr (requires { typename T::serialize_context; }) {
                return [](const context_entry &self, json::string_writer &writer) {
                    auto key = reflect::type_name<T>();
                    writer.Key(key.data(), key.size());
                    if constexpr (std::is_empty_v<T>) {
                        writer.Bool(true);
                    } else {
                        json::serialize(self.get<T>(), writer);
                    }
                };
            }
            return nullptr;
        }

    public:
        template<typename T, typename ... Ts>
        context_entry(std::in_place_type_t<T> tag, Ts && ... args)
            : value{tag, std::forward<Ts>(args) ...}
            , serialize_fun{make_serialize_fun<std::remove_cvref_t<T>>()}
        {}

        template<typename T>
        unwrapped_t<T> &get() {
            return unwrap_value(std::any_cast<T &>(value));
        }

        template<typename T>
        const unwrapped_t<T> &get() const {
            return unwrap_value(std::any_cast<const T &>(value));
        }

        bool serializable() const {
            return serialize_fun != nullptr;
        }

        void serialize(json::string_writer &writer) const {
            std::invoke(serialize_fun, *this, writer);
        }
    };

    class effect_context {
    private:
        std::unordered_map<std::type_index, context_entry> m_entries;

        template<typename T>
        static std::type_index key() {
            return std::type_index{typeid(std::remove_cvref_t<T>)};
        }

        template<typename T>
        static const T &default_value() {
            static const T value;
            return value;
        }
    
    public:
        template<typename T>
        unwrapped_t<T> &add() {
            auto [it, inserted] = m_entries.try_emplace(key<T>(), std::in_place_type<T>);
            return it->second.template get<T>();
        }

        template<typename T, typename ... Ts>
        unwrapped_t<T> &set(Ts && ... args) {
            auto [it, inserted] = m_entries.insert_or_assign(key<T>(), context_entry{std::in_place_type<T>, std::forward<Ts>(args) ...});
            return it->second.template get<T>();
        }

        template<typename T>
        bool contains() const {
            return m_entries.contains(key<T>());
        }

        template<typename T>
        const unwrapped_t<T> &get() const {
            auto it = m_entries.find(key<T>());
            if (it != m_entries.end()) {
                return it->second.template get<T>();
            }
            return default_value<unwrapped_t<T>>();
        }

        template<typename T> requires std::is_trivially_copyable_v<T>
        unwrapped_t<T> get() const {
            auto it = m_entries.find(key<T>());
            if (it != m_entries.end()) {
                return it->second.template get<T>();
            }
            return {};
        }
        
        void serialize(json::string_writer &writer) const {
            bool empty = true;
            for (const auto &[type, entry] : m_entries) {
                if (entry.serializable()) {
                    if (empty) {
                        empty = false;
                        writer.StartObject();
                    }
                    entry.serialize(writer);
                }
            }
            if (empty) {
                writer.Null();
            } else {
                writer.EndObject();
            }
        }
    };

}

namespace json {

    template<typename Context>
    struct serializer<banggame::effect_context, Context> {
        static void write(const banggame::effect_context &ctx, string_writer &writer) {
            ctx.serialize(writer);
        }
    };

}

#endif