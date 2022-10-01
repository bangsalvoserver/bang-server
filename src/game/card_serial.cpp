#include "card_serial.h"
#include "game.h"

namespace json {

Json::Value serializer<banggame::card *, banggame::game>::operator()(banggame::card *card) const {
    if (card) {
        return card->id;
    } else {
        return Json::nullValue;
    }
}

Json::Value serializer<banggame::player *, banggame::game>::operator()(banggame::player *player) const {
    if (player) {
        return player->id;
    } else {
        return Json::nullValue;
    }
}

banggame::card *deserializer<banggame::card *, banggame::game>::operator()(const Json::Value &value) const {
    if (value.isNull()) {
        return nullptr;
    } else {
        return context.find_card(value.asInt());
    }
}

banggame::player *deserializer<banggame::player *, banggame::game>::operator()(const Json::Value &value) const {
    if (value.isNull()) {
        return nullptr;
    } else {
        return context.find_player(value.asInt());
    }
}

}