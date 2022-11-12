#include "card_serial.h"
#include "game.h"

namespace json {

template<> Json::Value serializer<banggame::card *, banggame::game_table>::operator()(banggame::card *card) const {
    if (card) {
        return card->id;
    } else {
        return Json::nullValue;
    }
}

template<> Json::Value serializer<not_null<banggame::card *>, banggame::game_table>::operator()(not_null<banggame::card *> card) const {
    return card->id;
}

template<> Json::Value serializer<banggame::player *, banggame::game_table>::operator()(banggame::player *player) const {
    if (player) {
        return player->id;
    } else {
        return Json::nullValue;
    }
}

template<> Json::Value serializer<not_null<banggame::player *>, banggame::game_table>::operator()(not_null<banggame::player *> player) const {
    return player->id;
}

template<> banggame::card *deserializer<banggame::card *, banggame::game_table>::operator()(const Json::Value &value) const {
    if (value.isInt()) {
        return context.find_card(value.asInt());
    } else {
        return nullptr;
    }
}

template<> not_null<banggame::card *> deserializer<not_null<banggame::card *>, banggame::game_table>::operator()(const Json::Value &value) const {
    return context.find_card(value.asInt());
}

template<> banggame::player *deserializer<banggame::player *, banggame::game_table>::operator()(const Json::Value &value) const {
    if (value.isInt()) {
        return context.find_player(value.asInt());
    } else {
        return nullptr;
    }
}

template<> not_null<banggame::player *> deserializer<not_null<banggame::player *>, banggame::game_table>::operator()(const Json::Value &value) const {
    return context.find_player(value.asInt());
}

}

namespace banggame {

card_format_id::card_format_id(not_null<card *> value)
    : name(value->name)
    , sign(value->sign) {}

}