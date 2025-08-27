#ifndef __EXPANSION_SET_H__
#define __EXPANSION_SET_H__

#include "cards/bang_cards.h"

#include "utils/parse_string.h"

namespace banggame {

    inline bool validate_expansions(const expansion_set &expansions) {
        return rn::all_of(expansions, [&](banggame::ruleset_ptr ruleset) {
            return ruleset->is_valid_with(expansions);
        });
    }

}

namespace json {

    template<typename Context>
    struct serializer<banggame::ruleset_ptr, Context> {
        static void write(banggame::ruleset_ptr value, string_writer &writer) {
            serialize(value->name, writer);
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
                auto it = banggame::bang_cards.expansions.find(name);
                if (it != banggame::bang_cards.expansions.end() && it->second.expansion) {
                    result.insert(it->second.expansion);
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
            for (const auto &[name, data] : banggame::bang_cards.expansions) {
                if (data.expansion && value.contains(data.expansion)) {
                    if (!ret.empty()) {
                        ret += ' ';
                    }
                    ret.append(name);
                }
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
                auto it = banggame::bang_cards.expansions.find(str.substr(0, pos));
                if (it != banggame::bang_cards.expansions.end() && it->second.expansion) {
                    result.insert(it->second.expansion);
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