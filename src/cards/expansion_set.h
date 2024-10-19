#ifndef __EXPANSION_SET_H__
#define __EXPANSION_SET_H__

#include "cards/card_data.h"

#include "utils/parse_string.h"

namespace banggame {

    inline bool validate_expansions(const expansion_set &expansions) {
        return rn::all_of(expansions, [&](const banggame::ruleset_vtable *ruleset) {
            return ruleset->is_valid_with(expansions);
        });
    }

}

namespace json {

    template<typename Context> struct serializer<const banggame::ruleset_vtable *, Context> {
        json operator()(const banggame::ruleset_vtable *value) const {
            return value->name;
        }
    };

    template<typename Context> struct deserializer<const banggame::ruleset_vtable *, Context> {
        const banggame::ruleset_vtable *operator ()(const json &value) const {
            if (!value.is_string()) {
                throw deserialize_error("Cannot deserialize ruleset_vtable");
            }

            std::string_view name = value.get<std::string_view>();
            auto it = rn::find(banggame::all_cards.expansions, name, &banggame::ruleset_vtable::name);
            if (it == banggame::all_cards.expansions.end()) {
                throw deserialize_error(std::format("Invalid ruleset_vtable name: {}", name));
            }

            return *it;
        }
    };

}

namespace std {

    template<> struct formatter<banggame::expansion_set> : formatter<std::string_view> {
        static string expansions_to_string(banggame::expansion_set value) {
            std::string ret;
            for (const banggame::ruleset_vtable *ruleset : banggame::all_cards.expansions) {
                if (value.contains(ruleset)) {
                    if (!ret.empty()) {
                        ret += ' ';
                    }
                    ret.append(ruleset->name);
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
                auto it = rn::find(banggame::all_cards.expansions, str.substr(0, pos), &banggame::ruleset_vtable::name);
                if (it != banggame::all_cards.expansions.end()) {
                    result.insert(*it);
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