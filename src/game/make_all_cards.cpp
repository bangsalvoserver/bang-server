#include "make_all_cards.h"

#include <stdexcept>
#include <regex>

#include <json/json.h>
#include <fmt/core.h>

#include "utils/resource.h"
#include "utils/unpacker.h"

#include "holders.h"

DECLARE_RESOURCE(bang_cards_json)

namespace banggame {

    using namespace enums::flag_operators;

    struct invalid_effect : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    template<enums::reflected_enum E> E string_to_enum_or_throw(std::string_view str) {
        if (str.empty()) {
            return E{};
        } else if (auto value = enums::from_string<E>(str)) {
            return *value;
        } else {
            throw invalid_effect(fmt::format("Invalid {}: {}", enums::enum_name_v<E>, str));
        }
    }

    static short string_to_int_or_zero(const std::string &str) {
        return str.empty() ? 0 : std::stoi(str);
    }

    static card_expansion_type get_expansion(const Json::Value &value) {
        if (value.isMember("expansion")) {
            return string_to_enum_or_throw<card_expansion_type>(value["expansion"].asString());
        }
        return {};
    }

    static effect_list make_effects_from_json(const Json::Value &json_effects) {
        static const std::regex effect_string_regex(
            "^\\s*(\\w+)" // type
            "(?:\\s*\\((\\d+)\\))?" // effect_value
            "(?:\\s*(\\w+)\\s*)?" // target
            "([\\w\\s]*?)" // player_filter
            "(?:\\s*\\|\\s*([\\w\\s]+))?\\s*$" // card_filter
        );
        effect_list ret;
        for (const auto &json_effect : json_effects) {
            std::string str = json_effect.asString();
            std::smatch match;
            if (std::regex_match(str, match, effect_string_regex)) {
                effect_holder effect;
                effect.type = string_to_enum_or_throw<effect_type>(match.str(1));
                effect.effect_value = string_to_int_or_zero(match.str(2));
                effect.target = string_to_enum_or_throw<play_card_target_type>(match.str(3));
                effect.player_filter = string_to_enum_or_throw<target_player_filter>(match.str(4));
                effect.card_filter = string_to_enum_or_throw<target_card_filter>(match.str(5));

                if (bool(effect.player_filter)) {
                    switch (effect.target) {
                    case play_card_target_type::player:
                    case play_card_target_type::conditional_player:
                    case play_card_target_type::card:
                        break;
                    default:
                        throw invalid_effect(fmt::format("Target type {} cannot have a player filter", enums::to_string(effect.target)));
                    }
                }

                if (bool(effect.card_filter) && effect.target != play_card_target_type::card) {
                    throw invalid_effect(fmt::format("Target type {} cannot have a card filter", enums::to_string(effect.target)));
                }

                ret.push_back(effect);
            } else {
                throw invalid_effect(fmt::format("Invalid effect string: {}", str));
            }
        }

        return ret;
    }

    static equip_list make_equips_from_json(const Json::Value &json_equips) {
        static const std::regex equip_string_regex(
            "^\\s*(\\w+)" // type
            "(?:\\s*\\((\\d+)\\))?\\s*$" // effect_value
        );
        equip_list ret;
        for (const auto &json_equip : json_equips) {
            std::string str = json_equip.asString();
            std::smatch match;
            if (std::regex_match(str, match, equip_string_regex)) {
                equip_holder equip;
                equip.type = string_to_enum_or_throw<equip_type>(match.str(1));
                equip.effect_value = string_to_int_or_zero(match.str(2));

                ret.push_back(equip);
            } else {
                throw invalid_effect(fmt::format("Invalid equip string: {}", str));
            }
        }
        return ret;
    }

    tag_list make_tags_from_json(const Json::Value &json_tags) {
        static const std::regex tag_string_regex(
            "^\\s*(\\w+)" // type
            "(?:\\s*\\((\\d+)\\))?\\s*$" // tag_value
        );
        tag_list ret;
        for (const auto &json_tag : json_tags) {
            std::string str = json_tag.asString();
            std::smatch match;
            if (std::regex_match(str, match, tag_string_regex)) {
                tag_holder tag;
                tag.type = string_to_enum_or_throw<tag_type>(match.str(1));
                tag.tag_value = string_to_int_or_zero(match.str(2));

                ret.push_back(tag);
            } else {
                throw invalid_effect(fmt::format("Invalid tag string: {}", str));
            }
        }
        return ret;
    }
    
    static void make_all_effects(card_data &out, const Json::Value &json_card) {
        out.name = json_card["name"].asString();
        out.image = json_card["image"].asString();
        try {
            if (json_card.isMember("effects")) {
                out.effects = make_effects_from_json(json_card["effects"]);
            }
            if (json_card.isMember("responses")) {
                out.responses = make_effects_from_json(json_card["responses"]);
            }
            if (json_card.isMember("optional")) {
                out.optionals = make_effects_from_json(json_card["optional"]);
            }
            if (json_card.isMember("equip_target")) {
                out.equip_target = string_to_enum_or_throw<target_player_filter>(json_card["equip_target"].asString());
            }
            if (json_card.isMember("equip")) {
                out.equips = make_equips_from_json(json_card["equip"]);
            }
            if (json_card.isMember("modifier")) {
                out.modifier = string_to_enum_or_throw<card_modifier_type>(json_card["modifier"].asString());
            }
            if (json_card.isMember("multitarget")) {
                out.multi_target_handler.type = string_to_enum_or_throw<mth_type>(json_card["multitarget"].asString());
            }
            if (json_card.isMember("tags")) {
                out.tags = make_tags_from_json(json_card["tags"]);
            }
        } catch (const invalid_effect &e) {
            throw std::runtime_error(fmt::format("{}: {}", out.name, e.what()));
        }
    }

    all_cards_t::all_cards_t() {
        using namespace enums::flag_operators;

        Json::Value json_cards;
        {
            auto bang_cards_resource = GET_RESOURCE(bang_cards_json);
            std::stringstream ss(std::string(bang_cards_resource.data, bang_cards_resource.length));
            ss >> json_cards;
        }

        const auto is_disabled = [](const Json::Value &value) {
            return value.isMember("disabled") && value["disabled"].asBool();
        };

        for (const auto &json_card : json_cards["main_deck"]) {
            if (is_disabled(json_card)) continue;
            card_data c;
            c.deck = card_deck_type::main_deck;
            c.expansion = get_expansion(json_card);
            make_all_effects(c, json_card);
            c.color = string_to_enum_or_throw<card_color_type>(json_card["color"].asString());
            for (const auto &json_sign : json_card["signs"]) {
                static const std::regex sign_regex("^\\s*([\\w\\d]+)\\s+(\\w+)\\s*$");

                std::string str = json_sign.asString();
                std::smatch match;
                if (std::regex_match(str, match, sign_regex)) {
                    const auto &rank_letters = enums::enum_data_array_v<card_rank>;
                    if (auto it = std::ranges::find(rank_letters, match.str(1)); it != rank_letters.end()) {
                        c.sign.rank = enums::index_to<card_rank>(it - rank_letters.begin());
                    } else {
                        throw invalid_effect(fmt::format("Invalid card_rank: {}", match.str(1)));
                    }
                    c.sign.suit = string_to_enum_or_throw<card_suit>(match.str(2));
                    
                    deck.push_back(c);
                } else {
                    throw invalid_effect(fmt::format("Invalid sign string: {}", str));
                }
            }
        }

        for (const auto &json_character : json_cards["character"]) {
            if (is_disabled(json_character)) continue;
            card_data c;
            c.deck = card_deck_type::character;
            c.expansion = get_expansion(json_character);
            make_all_effects(c, json_character);
            characters.push_back(c);
        }

        for (const auto &json_card : json_cards["goldrush"]) {
            if (is_disabled(json_card)) continue;
            card_data c;
            c.deck = card_deck_type::goldrush;
            c.expansion = card_expansion_type::goldrush;
            make_all_effects(c, json_card);
            c.color = string_to_enum_or_throw<card_color_type>(json_card["color"].asString());
            if (c.has_tag(tag_type::hidden)) {
                hidden.push_back(c);
            } else {
                int count = json_card.isMember("count") ? json_card["count"].asInt() : 1;
                for (int i=0; i<count; ++i) {
                    goldrush.push_back(c);
                }
            }
        }

        for (const auto &json_card : json_cards["highnoon"]) {
            if (is_disabled(json_card)) continue;
            card_data c;
            c.deck = card_deck_type::highnoon;
            c.expansion = card_expansion_type::highnoon | get_expansion(json_card);
            make_all_effects(c, json_card);
            if (c.has_tag(tag_type::hidden)) {
                hidden.push_back(c);
            } else {
                highnoon.push_back(c);
            }
        }

        for (const auto &json_card : json_cards["fistfulofcards"]) {
            if (is_disabled(json_card)) continue;
            card_data c;
            c.deck = card_deck_type::fistfulofcards;
            c.expansion = card_expansion_type::fistfulofcards;
            make_all_effects(c, json_card);
            if (c.has_tag(tag_type::hidden)) {
                hidden.push_back(c);
            } else {
                fistfulofcards.push_back(c);
            }
        }

        for (const auto &json_card : json_cards["wildwestshow"]) {
            if (is_disabled(json_card)) continue;
            card_data c;
            c.deck = card_deck_type::wildwestshow;
            c.expansion = card_expansion_type::wildwestshow | get_expansion(json_card);
            make_all_effects(c, json_card);
            wildwestshow.push_back(c);
        }

        for (const auto &json_card : json_cards["specials"]) {
            if (is_disabled(json_card)) continue;
            card_data c;
            c.expansion = get_expansion(json_card);
            make_all_effects(c, json_card);
            if (c.has_tag(tag_type::hidden)) {
                hidden.push_back(c);
            } else {
                specials.push_back(c);
            }
        }
    };

}