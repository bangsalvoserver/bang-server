#ifndef __EXPANSION_SET_H__
#define __EXPANSION_SET_H__

#include "cards/card_data.h"

#include "utils/parse_string.h"

template<> struct std::formatter<banggame::expansion_set> : std::formatter<std::string_view> {
    static std::string expansions_to_string(banggame::expansion_set value) {
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

    auto format(banggame::expansion_set value, std::format_context &ctx) const {
        return std::formatter<std::string_view>::format(expansions_to_string(value), ctx);
    }
};

template<typename Context> struct json::deserializer<banggame::expansion_set, Context> {
    banggame::expansion_set operator()(const json &value) const {
        if (!value.is_array()) {
            throw deserialize_error("Cannot deserialize expansion_set");
        }

        banggame::expansion_set result;
        for (const banggame::ruleset_vtable *ruleset : banggame::all_cards.expansions) {
            if (rn::contains(value, ruleset->name, [](const json &name) { return name.get<std::string_view>(); })) {
                result.insert(ruleset);
            }
        }
        return result;
    }
};

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
        return result;
    }
};

#endif