#include "patka.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_patka::on_enable(card_ptr target_card, player_ptr target) {
        // Efekt odhození globálních karet je řízen přímo YAML filtrem all_players
    }
}