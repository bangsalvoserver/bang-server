#include <regex>
#include <sstream>
#include <fstream>
#include <iostream>

#include <yaml-cpp/yaml.h>
#include <fmt/core.h>

void set_expansion(YAML::Node &card, const std::string &str) {
    if (auto expansion = card["expansion"]) {
        expansion = fmt::format("{} {}", str, expansion.as<std::string>());
    } else {
        expansion = str;
    }
}

bool is_hidden(const YAML::Node &card) {
    if (auto &tags = card["tags"]) {
        for (const auto &str : tags) {
            if (str.as<std::string>() == "hidden") {
                return true;
            }
        }
    }
    return false;
}

std::string parse_sign(const std::string &sign) {
    static const std::regex sign_regex {
        "^\\s*([\\w\\d]+)\\s*(\\w+)\\s*$"
    };
    std::smatch match;
    if (std::regex_match(sign, match, sign_regex)) {
        return fmt::format("card_suit::{0}, card_rank::rank_{1}", match.str(2), match.str(1));
    } else {
        throw std::runtime_error(fmt::format("Invalid sign string: {}", sign));
    }
}

std::string enum_flags(const std::string &sign, const std::string &enum_name) {
    std::string ret;
    size_t begin = sign.find_first_not_of(" \t");
    while (begin != std::string::npos) {
        size_t end = sign.find_first_of(" \t", begin);
        if (end == std::string::npos) {
            end = sign.size();
        }
        if (!ret.empty()) {
            ret += " | ";
        }
        ret += fmt::format("{}::{}", enum_name, sign.substr(begin, end - begin));
        begin = sign.find_first_not_of(" \t", end);
    }
    return ret;
}

void parse_effects(std::ostream &out, const YAML::Node &list, const std::string &name) {
    static const std::regex effect_regex {
        "^\\s*(\\w+)" // type
        "(?:\\s*\\((-?\\d+)\\))?" // effect_value
        "(?:\\s*(\\w+)\\s*)?" // target
        "([\\w\\s]*?)" // player_filter
        "(?:\\s*\\|\\s*([\\w\\s]+))?\\s*$", // card_filter
    };
    out << fmt::format(",\n      .{} {{\n", name);
    for (auto it = list.begin(); it != list.end();) {
        const std::string effect = it->as<std::string>();
        std::smatch match;
        if (!std::regex_match(effect, match, effect_regex)) {
            throw std::runtime_error(fmt::format("Invalid effect string: {}", effect));
        }
        auto type = match.str(1);
        auto effect_value = match.str(2);
        auto target = match.str(3);
        auto player_filter = match.str(4);
        auto card_filter = match.str(5);

        out << "        {\n";

        if (!target.empty()) {
            out << fmt::format("          .target {{target_type::{}}},\n", target);
        }
        if (!player_filter.empty()) {
            if (target != "player" && target != "conditional_player" && target != "card") {
                throw std::runtime_error(fmt::format("Invalid effect string: {0}\nPlayer filter not allowed with {1}", effect, target));
            }
            out << fmt::format("          .player_filter {{{}}},\n", enum_flags(player_filter, "target_player_filter"));
        }
        if (!card_filter.empty()) {
            if (target != "card") {
                throw std::runtime_error(fmt::format("Invalid effect string: {0}\nCard filter not alowed with {1}", effect, target));
            }
            out << fmt::format("          .card_filter {{{}}},\n", enum_flags(card_filter, "target_card_filter"));
        }
        if (!effect_value.empty()) {
            out << fmt::format("          .effect_value {{{}}},\n", effect_value);
        }
        out << fmt::format("          .type {{effect_type::{}}}\n        }}", type);
        if (++it == list.end()) {
            out << "\n";
        } else {
            out << ",\n";
        }
    }
    out << "      }";
}

void parse_effect_simple(std::ostream &out, const YAML::Node &list,
    const std::string &name, const std::string &value_name, const std::string &type_name)
{
    static const std::regex effect_regex {
        "^\\s*(\\w+)" // type
        "(?:\\s*\\((-?\\d+)\\))?\\s*$" // effect_value
    };

    out << fmt::format(",\n      .{} {{\n", name);
    for (auto it = list.begin(); it != list.end();) {
        const std::string effect = it->as<std::string>();
        std::smatch match;
        if (!std::regex_match(effect, match, effect_regex)) {
            throw std::runtime_error(fmt::format("Invalid {0} string: {1}", name, effect));
        }

        std::string type = match.str(1);
        std::string value = match.str(2);

        out << "        {\n";
        if (!value.empty()) {
            out << fmt::format("          .{0} {{{1}}},\n", value_name, value);
        }
        out << fmt::format("          .type {{{0}::{1}}}\n        }}", type_name, type);
        if (++it == list.end()) {
            out << "\n";
        } else {
            out << ",\n";
        }
    }
    out << "      }";
}

void parse_all_effects(std::ostream &out, YAML::Node card) {
    try {
        out << fmt::format("    {{\n      .name {{\"{}\"}}", card["name"].as<std::string>());
        if (auto image = card["image"]) {
            out << fmt::format(",\n      .image {{\"{}\"}}", image.as<std::string>());
        }
        if (auto effects = card["effects"]) {
            parse_effects(out, effects, "effects");
        }
        if (auto responses = card["responses"]) {
            parse_effects(out, responses, "responses");
        }
        if (auto optional = card["optional"]) {
            parse_effects(out, optional, "optionals");
        }
        if (auto equip = card["equip"]) {
            parse_effect_simple(out, equip, "equips", "effect_value", "equip_type");
        }
        if (auto tags = card["tags"]) {
            parse_effect_simple(out, tags, "tags", "tag_value", "tag_type");
        }
        if (auto expansion = card["expansion"]) {
            out << fmt::format(",\n      .expansion {{{}}}",
                enum_flags(expansion.as<std::string>(), "card_expansion_type"));
        }
        if (auto deck = card["deck"]) {
            out << fmt::format(",\n      .deck {{card_deck_type::{}}}", deck.as<std::string>());
        }
        if (auto modifier = card["modifier"]) {
            out << fmt::format(",\n      .modifier {{card_modifier_type::{}}}", modifier.as<std::string>());
        }
        if (auto mth_effect = card["mth_effect"]) {
            out << fmt::format(",\n      .mth_effect {{mth_type::{}}}", mth_effect.as<std::string>());
        }
        if (auto mth_response = card["mth_response"]) {
            out << fmt::format(",\n      .mth_response {{mth_type::{}}}", mth_response.as<std::string>());
        }
        if (auto equip_target = card["equip_target"]) {
            out << fmt::format(",\n      .equip_target {{target_player_filter::{}}}", equip_target.as<std::string>());
        }
        if (auto color = card["color"]) {
            out << fmt::format(",\n      .color {{card_color_type::{}}}", color.as<std::string>());
        }
    } catch (const std::runtime_error &error) {
        throw std::runtime_error(fmt::format("Error in card {0}:\n{1}", card["name"].as<std::string>(), error.what()));
    }
}

void parse_file(std::ostream &out, const YAML::Node &data) {
    std::vector<YAML::Node> hidden_cards;

    out <<
        "// AUTO GENERATED FILE\n\n"
        "#include \"game/card_data.h\"\n\n"
        "namespace banggame {\n\n"
        "using namespace enums::flag_operators;\n\n"
        "const all_cards_t all_cards {\n";
    
    out << "  .deck {\n";
    for (YAML::Node card : data["main_deck"]) {
        card["deck"] = "main_deck";
        for (const YAML::Node &sign : card["signs"]) {
            parse_all_effects(out, card);
            out << fmt::format(",\n      .sign {{{}}}\n    }},\n", parse_sign(sign.as<std::string>()));
        }
    }
    out << "  },\n";

    out << "  .characters {\n";
    for (YAML::Node card : data["character"]) {
        card["deck"] = "character";
        parse_all_effects(out, card);
        out << "\n    },\n";
    }
    out << "  },\n";

    out << "  .goldrush {\n";
    for (YAML::Node card : data["goldrush"]) {
        set_expansion(card, "goldrush");
        card["deck"] = "goldrush";
        int count = card["count"].as<int>(1);
        for (int i=0; i<count; ++i) {
            if (is_hidden(card)) {
                hidden_cards.push_back(card);
            } else {
                parse_all_effects(out, card);
                out << "\n    },\n";
            }
        }
    }
    out << "  },\n";

    out << "  .highnoon {\n";
    for (YAML::Node card : data["highnoon"]) {
        set_expansion(card, "highnoon");
        card["deck"] = "highnoon";
        if (is_hidden(card)) {
            hidden_cards.push_back(card);
        } else {
            parse_all_effects(out, card);
            out << "\n    },\n";
        }
    }
    out << "  },\n";

    out << "  .fistfulofcards {\n";
    for (YAML::Node card : data["fistfulofcards"]) {
        set_expansion(card, "fistfulofcards");
        card["deck"] = "fistfulofcards";
        if (is_hidden(card)) {
            hidden_cards.push_back(card);
        } else {
            parse_all_effects(out, card);
            out << "\n    },\n";
        }
    }
    out << "  },\n";

    out << "  .wildwestshow {\n";
    for (YAML::Node card : data["wildwestshow"]) {
        set_expansion(card, "wildwestshow");
        card["deck"] = "wildwestshow";
        if (is_hidden(card)) {
            hidden_cards.push_back(card);
        } else {
            parse_all_effects(out, card);
            out << "\n    },\n";
        }
    }
    out << "  },\n";

    out << "  .specials {\n";
    for (YAML::Node card : data["specials"]) {
        if (is_hidden(card)) {
            hidden_cards.push_back(card);
        } else {
            parse_all_effects(out, card);
            out << "\n    },\n";
        }
    }
    out << "  },\n";

    out << "  .hidden {\n";
    for (YAML::Node card : hidden_cards) {
        parse_all_effects(out, card);
        out << "\n    },\n";
    }
    out << "  },\n";

    out << "};\n\n}\n";
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << fmt::format("Usage: {} bang_cards.yml bang_cards.cpp\n", argv[1]);
        return 1;
    }

    std::stringstream out;
    parse_file(out, YAML::LoadFile(argv[1]));

    std::ofstream ofs(argv[2]);
    ofs << out.str();

    return 0;
}