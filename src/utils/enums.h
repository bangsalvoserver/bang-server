#ifndef __ENUMS_H__
#define __ENUMS_H__

#include <format>
#include <meta>

#include "json_serial.h"
#include "static_map.h"
#include "visit_indexed.h"
#include "misc.h"

namespace enums {

    template<typename T> concept enumeral = std::is_enum_v<T>;

    template<enumeral auto E> struct enum_tag_t {};

    template<enumeral auto E> inline constexpr enum_tag_t<E> enum_tag;
    
    template<enumeral T> inline constexpr auto enumerators = std::define_static_array(std::meta::enumerators_of(^^T));

    template<enumeral T> inline constexpr size_t enum_count = enumerators<T>.size();

    template<enumeral T>
    consteval auto build_enum_values() {
        std::array<T, enum_count<T>> values;
        rn::copy(enumerators<T> | rv::transform(std::meta::extract<T>), values.begin());
        return values;
    }
    
    template<enumeral T>
    consteval bool is_linear_enum() {
        size_t i = 0;
        for (std::meta::info e : enumerators<T>) {
            if (static_cast<size_t>(std::meta::extract<T>(e)) != i) return false;
            ++i;
        }
        return true;
    }

    template<enumeral T>
    class enum_values_t {
    public:
        class iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = ptrdiff_t;
            using value_type = T;
            using reference = value_type;
    
        public:
            constexpr iterator() = default;

            explicit constexpr iterator(size_t index): m_index{index} {}
            
            constexpr value_type operator *() const {
                if constexpr (is_linear_enum<T>()) {
                    return static_cast<T>(m_index);
                } else {
                    static constexpr auto values = build_enum_values<T>();
                    return values[m_index];
                }
            }

            constexpr iterator &operator ++ () { ++m_index; return *this; }
            constexpr iterator operator ++ (int) { auto copy = *this; ++m_index; return copy; }

            constexpr iterator &operator --() { --m_index; return *this; }
            constexpr iterator operator -- (int) { auto copy = *this; --m_index; return copy; }

            constexpr iterator &operator += (difference_type n) { m_index += n; return *this; }
            constexpr iterator &operator -= (difference_type n) { m_index -= n; return *this; }

            constexpr value_type operator[](difference_type n) const { return *((*this) + n); };

            friend constexpr auto operator <=> (const iterator &lhs, const iterator &rhs) = default;

            friend constexpr iterator operator + (const iterator &lhs, difference_type n) { return iterator{ lhs.m_index + n }; }
            friend constexpr iterator operator + (difference_type n, const iterator &i) { return i + n; }

            friend constexpr iterator operator - (const iterator &lhs, difference_type n) { return iterator{ lhs.m_index - n }; }
            friend constexpr difference_type operator - (const iterator &lhs, const iterator &rhs) { return lhs.m_index - rhs.m_index; }
        
        private:
            size_t m_index = 0;
        };
    
        using reverse_iterator = std::reverse_iterator<iterator>;
        using value_type = T;
        using size_type = size_t;
    
        constexpr iterator begin() const { return iterator{}; }
        constexpr iterator end() const { return iterator{ enum_count<T> }; }

        constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }
        constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }

        constexpr value_type operator[] (size_t index) const { return *(begin() + index); }

        constexpr size_type size() const { return enum_count<T>; }
    };

    template<enumeral T> inline constexpr enum_values_t<T> enum_values;

    template<enumeral T>
    constexpr size_t indexof(T value) {
        for (size_t i=0; i<enum_values<T>.size(); ++i) {
            if (enum_values<T>[i] == value) {
                return i;
            }
        }
        throw std::out_of_range("invalid enum index");
    }
    
    template<enumeral T> requires (is_linear_enum<T>())
    constexpr size_t indexof(T value) {
        size_t result = static_cast<size_t>(value);
        assert((result < enum_count<T>) && "enum index is out of bounds");
        return result;
    }

    template<enumeral T>
    constexpr bool is_between(T value, T min, T max) {
        size_t index = indexof(value);
        return index >= indexof(min) && index <= indexof(max);
    }

    template<enumeral T>
    consteval auto build_enum_names() {
        std::array<std::string_view, enum_count<T>> names;
        rn::copy(enumerators<T> | rv::transform(std::meta::identifier_of), names.begin());
        return names;
    }

    template<enumeral T>
    inline constexpr auto enum_names = build_enum_names<T>();

    template<enumeral T>
    constexpr std::string_view to_string(T input) {
        return enum_names<T>[indexof(input)];
    }

    template<enumeral T>
    consteval auto build_names_to_values_map() {
        return []<size_t ... Is>(std::index_sequence<Is ...>) {
            return utils::make_static_map<std::string_view, T>({
                { to_string(enum_values<T>[Is]), enum_values<T>[Is] } ...
            });
        }(std::make_index_sequence<enum_count<T>>());
    }

    template<enumeral T>
    inline constexpr auto names_to_values_map = build_names_to_values_map<T>();

    template<enumeral T>
    constexpr std::optional<T> from_string(std::string_view str) {
        if (auto it = names_to_values_map<T>.find(str); it != names_to_values_map<T>.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    template<typename RetType, typename Visitor, enumeral ... E>
    RetType visit_enum(Visitor &&visitor, E ... values) {
        return utils::visit_indexed_r<RetType, enum_count<E> ...>([&](auto ... Is) {
            return std::invoke(std::forward<Visitor>(visitor), enum_tag<enum_values<E>[Is]> ...);
        }, indexof(values) ...);
    }

    template<typename Visitor, enumeral E>
    decltype(auto) visit_enum(Visitor &&visitor, E value) {
        using return_type = std::invoke_result_t<Visitor, enum_tag_t<enum_values<E>[0]>>;
        return visit_enum<return_type>(std::forward<Visitor>(visitor), value);
    }
    
}

namespace json {

    template<enums::enumeral T, typename Context>
    struct deserializer<T, Context> {
        static T read(const json &value) {
            if (!value.IsString()) {
                throw deserialize_error(std::format("Cannot deserialize {}: value is not a string", type_name<T>));
            }
            std::string_view str{value.GetString(), value.GetStringLength()};
            if (auto ret = enums::from_string<T>(str)) {
                return *ret;
            } else {
                throw deserialize_error(std::format("Invalid {} value: {}", type_name<T>, str));
            }
        }
    };

    template<enums::enumeral T, typename Context>
    struct serializer<T, Context> {
        static void write(const T &value, string_writer &writer) {
            serialize(enums::to_string(value), writer);
        }
    };

}

namespace std {

    template<enums::enumeral E>
    struct formatter<E> : formatter<std::string_view> {
        auto format(E value, format_context &ctx) const {
            return formatter<std::string_view>::format(enums::to_string(value), ctx);
        }
    };
    
}

#endif