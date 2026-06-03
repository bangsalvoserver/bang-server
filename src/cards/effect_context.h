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
        template<typename T> requires (!std::is_same_v<std::remove_cvref_t<T>, context_entry>)
        context_entry(T &&value)
            : value{std::forward<T>(value)}
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
            (*serialize_fun)(*this, writer);
        }
    };

    class effect_context {
    private:
        std::vector<context_entry> m_entries;
    
    public:
        template<typename T>
        void add(T &&entry) {
            m_entries.emplace_back(std::forward<T>(entry));
        }

        void resize(size_t new_size) {
            assert(new_size <= size());
            m_entries.erase(m_entries.begin() + new_size, m_entries.end());
        }

        size_t size() const {
            return m_entries.size();
        }

        template<typename T>
        bool contains() const {
            return rn::contains(m_entries, typeid(T), &context_entry::type);
        }

        template<typename T> requires std::is_trivially_copyable_v<T>
        unwrapped_t<T> get() const {
            auto it = rn::find_last(m_entries, typeid(T), &context_entry::type).begin();
            if (it != m_entries.end()) {
                return it->template get<T>();
            }
            return {};
        }

        template<typename T>
        auto get_all() const {
            return m_entries
                | rv::filter([](const context_entry &entry) { return entry.type() == typeid(T); })
                | rv::transform([](const context_entry &entry) -> decltype(auto) { return entry.get<T>(); });
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

        effect_context get_serializable() const {
            effect_context ctx;
            for (const context_entry &entry : m_entries) {
                if (entry.serializable()) {
                    ctx.m_entries.push_back(entry);
                }
            }
            return ctx;
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