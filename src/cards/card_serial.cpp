#include "card_serial.h"

#include "game/game.h"

#include "cards/filters.h"

namespace banggame {
    bool player_is_bot(const player *origin) {
        return origin->is_bot();
    }

    bool card_is_equip(const card *origin_card) {
        return filters::is_equip_card(origin_card);
    }

    bool card_is_modifier(const card *origin_card) {
        return origin_card->is_modifier();
    }
}

namespace json {

template<> json serializer<banggame::card *, banggame::game_context>::operator()(banggame::card *card) const {
    if (card) {
        return card->id;
    } else {
        return {};
    }
}

template<> json serializer<not_null<banggame::card *>, banggame::game_context>::operator()(not_null<banggame::card *> card) const {
    return card->id;
}

template<> json serializer<banggame::player *, banggame::game_context>::operator()(banggame::player *player) const {
    if (player) {
        return player->id;
    } else {
        return {};
    }
}

template<> json serializer<not_null<banggame::player *>, banggame::game_context>::operator()(not_null<banggame::player *> player) const {
    return player->id;
}

template<> banggame::card *deserializer<banggame::card *, banggame::game_context>::operator()(const json &value) const {
    if (value.is_number_integer()) {
        return context.find_card(value.get<int>());
    } else {
        return nullptr;
    }
}

template<> not_null<banggame::card *> deserializer<not_null<banggame::card *>, banggame::game_context>::operator()(const json &value) const {
    return context.find_card(value.get<int>());
}

template<> banggame::player *deserializer<banggame::player *, banggame::game_context>::operator()(const json &value) const {
    if (value.is_number_integer()) {
        return context.find_player(value.get<int>());
    } else {
        return nullptr;
    }
}

template<> not_null<banggame::player *> deserializer<not_null<banggame::player *>, banggame::game_context>::operator()(const json &value) const {
    return context.find_player(value.get<int>());
}

template<> json serializer<banggame::serial::card_format, banggame::game_context>::operator()(banggame::serial::card_format value) const {
    if (value.card) {
        return serialize(banggame::card_format{
            .name = value.card->name,
            .sign = value.card->sign
        });
    } else {
        return json::object();
    }
}

template<> json serializer<banggame::serial::modifier_type, banggame::game_context>::operator()(banggame::serial::modifier_type value) const {
    if (value) {
        return std::string(value->name);
    } else {
        return "none";
    }
}

template<> json serializer<banggame::serial::mth_type, banggame::game_context>::operator()(banggame::serial::mth_type value) const {
    return std::string(value->name);
}

}