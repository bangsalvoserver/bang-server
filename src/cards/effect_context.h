#ifndef __EFFECT_CONTEXT_H__
#define __EFFECT_CONTEXT_H__

#include <any>
#include <vector>
#include <typeinfo>
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

        const std::type_info &type() const {
            return value.type();
        }

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
        std::vector<context_entry> m_entries;

        template<typename T>
        auto find(this auto &&self) {
            return rn::find(self.m_entries, typeid(T), &context_entry::type);
        }

        template<typename T>
        static const T &default_value() {
            static const T value;
            return value;
        }
    
    public:
        template<typename T>
        unwrapped_t<T> &add() {
            if (auto it = find<T>(); it != m_entries.end()) {
                return it->template get<T>();
            }
            return m_entries.emplace_back(std::in_place_type<T>).template get<T>();
        }

        template<typename T, typename ... Ts>
        unwrapped_t<T> &set(Ts && ... args) {
            context_entry entry{std::in_place_type<T>, std::forward<Ts>(args) ...};
            if (auto it = find<T>(); it != m_entries.end()) {
                *it = std::move(entry);
                return it->template get<T>();
            } else {
                return m_entries.emplace_back(std::move(entry)).get<T>();
            }
        }

        template<typename T>
        bool contains() const {
            return find<T>() != m_entries.end();
        }

        template<typename T>
        const unwrapped_t<T> &get() const {
            if (auto it = find<T>(); it != m_entries.end()) {
                return it->template get<T>();
            }
            return default_value<unwrapped_t<T>>();
        }

        template<typename T> requires std::is_trivially_copyable_v<T>
        unwrapped_t<T> get() const {
            if (auto it = find<T>(); it != m_entries.end()) {
                return it->template get<T>();
            }
            return {};
        }
        
        void serialize(json::string_writer &writer) const {
            bool empty = true;
            for (const context_entry &entry : m_entries) {
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