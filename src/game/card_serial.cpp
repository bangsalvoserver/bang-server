#include "card_serial.h"
#include "game.h"

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

}

namespace banggame {

card_format_id::card_format_id(card *value) {
    if (value) {
        name = value->name;
        sign = value->sign;
    }
}

}