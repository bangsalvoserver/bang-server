#ifndef __EXPANSION_SET_H__
#define __EXPANSION_SET_H__

#include "cards/card_fwd.h"

#include "utils/parse_string.h"

#include <limits>
#include <bit>

namespace banggame {

    uint64_t get_expansion_bit(ruleset_ptr value);
    ruleset_ptr get_expansion_by_name(std::string_view name);
    std::string_view get_expansion_name(ruleset_ptr value);
    bool validate_expansions(const expansion_set &expansions);

    using expansion_bitset = uint64_t;

    class expansion_set_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = ruleset_ptr;
        using pointer = value_type *;
        using reference = value_type;
    
    private:
        static constexpr int end_index = std::numeric_limits<expansion_bitset>::digits;

        expansion_bitset m_value = 0;
        int m_index = 0;

        expansion_set_iterator(expansion_bitset value, int index)
            : m_value{value}, m_index{index} {}
    
    public:
        expansion_set_iterator() = default;

        static expansion_set_iterator begin(expansion_bitset value) {
            return expansion_set_iterator{value, value == 0 ? end_index : std::countr_zero(value)};
        }

        static expansion_set_iterator end(expansion_bitset value) {
            return expansion_set_iterator{value, end_index};
        }
        
        bool operator == (const expansion_set_iterator &other) const = default;

        expansion_set_iterator &operator ++ () {
            auto shifted = m_value >> (m_index + 1);
            if (shifted == 0) {
                m_index = end_index;
            } else {
                m_index += 1 + std::countr_zero(shifted);
            }
            return *this;
        }

        expansion_set_iterator operator ++ (int) {
            auto copy = *this; ++(*this); return copy;
        }

        value_type operator *() const;
    };

    class expansion_set {
    private:
        expansion_bitset m_value = 0;
    
    public:
        using value_type = ruleset_ptr;
        using iterator = expansion_set_iterator;
        using const_iterator = expansion_set_iterator;

        void add(ruleset_ptr value) {
            m_value |= get_expansion_bit(value);
        }

        bool contains(ruleset_ptr value) const {
            return (m_value & get_expansion_bit(value)) != 0;
        }

        auto begin() const {
            return iterator::begin(m_value);
        }

        auto end() const {
            return iterator::end(m_value);
        }
    };

}

namespace json {

    template<typename Context>
    struct serializer<banggame::ruleset_ptr, Context> {
        static void write(banggame::ruleset_ptr value, string_writer &writer) {
            serialize(banggame::get_expansion_name(value), writer);
        }
    };

    template<typename Context>
    struct deserializer<banggame::expansion_set, Context> {
        static banggame::expansion_set read(const json &value) {
            if (!value.IsArray()) {
                throw deserialize_error("Cannot deserialize expansion_set");
            }

            banggame::expansion_set result;

            for (const auto &elem : value.GetArray()) {
                if (!elem.IsString()) {
                    throw deserialize_error("Cannot deserialize ruleset_ptr");
                }

                std::string_view name{elem.GetString(), elem.GetStringLength()};
                if (banggame::ruleset_ptr expansion = banggame::get_expansion_by_name(name)) {
                    result.add(expansion);
                }
            }

            return result;
        }
    };

}

namespace std {

    template<> struct formatter<banggame::expansion_set> : formatter<std::string_view> {
        static string expansions_to_string(banggame::expansion_set value) {
            std::string ret;
            for (banggame::ruleset_ptr expansion : value) {
                if (!ret.empty()) {
                    ret += ' ';
                }
                ret.append(banggame::get_expansion_name(expansion));
            }
            return ret;
        }

        auto format(banggame::expansion_set value, format_context &ctx) const {
            return formatter<std::string_view>::format(expansions_to_string(value), ctx);
        }
    };

}

namespace utils {

    template<> struct string_parser<banggame::expansion_set> {
        std::optional<banggame::expansion_set> operator()(std::string_view str) {
            constexpr std::string_view whitespace = " \t";
            banggame::expansion_set result;
            while (true) {
                size_t pos = str.find_first_not_of(whitespace);
                if (pos == std::string_view::npos) break;
                str = str.substr(pos);
                pos = str.find_first_of(whitespace);
                if (banggame::ruleset_ptr value = banggame::get_expansion_by_name(str.substr(0, pos))) {
                    result.add(value);
                } else {
                    return std::nullopt;
                }
                if (pos == std::string_view::npos) break;
                str = str.substr(pos);
            }
            if (banggame::validate_expansions(result)) {
                return result;
            }
            return std::nullopt;
        }
    };

}

#endif