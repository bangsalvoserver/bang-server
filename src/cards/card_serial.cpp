#include "card_serial.h"

#include "filter_enums.h"
#include "game_enums.h"

#include "game/game_net.h"

namespace json {

    template<typename Context> struct serializer<banggame::card *, Context> {
        json operator()(banggame::card *card) const {
            if (card) {
                return card->id;
            } else {
                return {};
            }
        }
    };

    template<> struct deserializer<banggame::card *, banggame::game_context> {
        const banggame::game_context &context;
        banggame::card *operator()(const json &value) const {
            if (value.is_number_integer()) {
                return context.find_card(value.get<int>());
            } else {
                return nullptr;
            }
        }
    };

    template<typename Context> struct serializer<not_null<banggame::card *>, Context> {
        json operator()(banggame::card *card) const {
            return card->id;
        }
    };

    template<> struct deserializer<not_null<banggame::card *>, banggame::game_context> {
        const banggame::game_context &context;
        banggame::card *operator()(const json &value) const {
            return context.find_card(value.get<int>());
        }
    };

    template<typename Context> struct serializer<banggame::player *, Context> {
        json operator()(banggame::player *player) const {
            if (player) {
                return player->id;
            } else {
                return {};
            }
        }
    };

    template<> struct deserializer<banggame::player *, banggame::game_context> {
        const banggame::game_context &context;
        banggame::player *operator()(const json &value) const {
            if (value.is_number_integer()) {
                return context.find_player(value.get<int>());
            } else {
                return nullptr;
            }
        }
    };

    template<typename Context> struct serializer<not_null<banggame::player *>, Context> {
        json operator()(banggame::player *player) const {
            return player->id;
        }
    };

    template<> struct deserializer<not_null<banggame::player *>, banggame::game_context> {
        const banggame::game_context &context;
        banggame::player *operator()(const json &value) const {
            return context.find_player(value.get<int>());
        }
    };

    template<typename Context> struct serializer<banggame::card_format, Context> {
        json operator()(banggame::card_format value) const {
            if (value.card) {
                return {
                    {"name", value.card->name},
                    {"sign", serialize(value.card->sign)}
                };
            } else {
                return json::object();
            }
        }
    };

    template<typename Context> struct serializer<banggame::serial::effect_type, Context> {
        json operator()(banggame::serial::effect_type value) const {
            return std::string(value->name);
        }
    };

    template<typename Context> struct serializer<banggame::serial::equip_type, Context> {
        json operator()(banggame::serial::equip_type value) const {
            return std::string(value->name);
        }
    };

    template<typename Context> struct serializer<banggame::serial::modifier_type, Context> {
        json operator()(banggame::serial::modifier_type value) const {
            if (value) {
                return std::string(value->name);
            } else {
                return {};
            }
        }
    };

    template<typename Context> struct serializer<banggame::serial::mth_type, Context> {
        json operator()(banggame::serial::mth_type value) const {
            if (value) {
                return std::string(value->name);
            } else {
                return {};
            }
        }
    };

}

namespace banggame {
    json::json game_net_manager::serialize_update(const game_update &update) const {
        return json::serialize(update, context());
    }

    game_action game_net_manager::deserialize_action(const json::json &value) const {
        return json::deserialize<game_action>(value, context());
    }
}