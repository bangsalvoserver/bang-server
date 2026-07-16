# Practical example: adding an entire expansion

This guide builds on the previous series (server architecture, frontend architecture, adding individual cards) by tackling the level above: **how to add an entire expansion**, i.e. a package of cards plus any new global rules, toggleable from the lobby.

Unlike adding a single card (which typically only touches `config/sets/*.yml` and possibly one file in `effects/`), an expansion touches **four distinct levels**:

1. **Data** — the YAML file(s) with the cards, and registering the expansion in the `bang_cards.yml` index.
2. **Ruleset** — a C++ file representing the "global rules" activated when the expansion is selected (not all expansions need one).
3. **Build system** — registering the new folder in the `CMakeLists.txt` files and the collector headers (`effects.h`/`target_types.h`).
4. **Frontend** — the expansion must appear in the lobby's options editor, with labels localized in every language, and its cards must have translated name/description.

---

## 1. How the server "knows" which expansions exist

The index file `src/config/bang_cards.yml` lists **all** known expansions, mapping each to its own card file (or to an empty dictionary if the expansion introduces no physical cards, only rules):

```yaml
base: !include sets/base.yml
dodgecity: !include sets/dodgecity.yml
valleyofshadows: !include sets/valleyofshadows.yml
armedanddangerous: !include sets/armedanddangerous.yml
...
ghost_cards: {}          # <- a "rule-only" expansion, no cards of its own
stickofdynamite: {}      # <- same
shadowgunslingers: {}    # <- same
```

For each key, `parse_bang_cards.py` generates an entry in the static `bang_cards.expansions` map, with a pointer to the corresponding `ruleset_vtable` (`GET_RULESET(name)`) — **except for `base`**, which is treated as a special case, always active and with no ruleset of its own. This means **every new expansion must have, somewhere in the C++ code, a registration**:

```cpp
DEFINE_RULESET(expansion_name, RulesetType)
```

even if it introduces no cards at all (like `ghost_cards`), because it's this macro that makes `GET_RULESET(expansion_name)` resolvable at compile time.

---

## 2. The "ruleset": where an expansion's global rules live

A YAML `!include` is used when the expansion introduces **cards with effects**. But an expansion often also (or only) introduces **a rule that always applies**, regardless of which cards get drawn — for example, in *Dodge City*, green cards get "deactivated" after their first use in a turn, regardless of which specific green card it is. This rule lives in the expansion folder's `ruleset.cpp` file, not in a single card.

Real, simplified example (`effects/dodgecity/ruleset.cpp`):

```cpp
struct ruleset_dodgecity {
    void on_apply(game_ptr game);
};
DEFINE_RULESET(dodgecity, ruleset_dodgecity)
```

```cpp
void ruleset_dodgecity::on_apply(game_ptr game) {
    // Any green card played gets deactivated for the rest of the turn...
    game->add_listener<event_type::on_equip_card>({nullptr, 5}, [=](player_ptr origin, player_ptr target, card_ptr target_card, const effect_context &ctx) {
        if (target_card->is_green()) {
            target_card->set_inactive(true);
            // ... track who activated it, to block reuse in the same turn
        }
    });

    // ...and reactivates at the end of the player's turn
    game->add_listener<event_type::on_turn_end>({nullptr, 5}, [=](player_ptr origin, bool skipped) {
        for (card_ptr target_card : origin->m_table) {
            target_card->set_inactive(false);
        }
    });

    // A player cannot play a green card already used this turn
    game->add_listener<event_type::check_play_card>(nullptr, [](player_ptr origin, card_ptr target_card, const effect_context &ctx) -> game_string {
        if (target_card->is_green() && target_card->inactive) {
            return {"ERROR_CARD_INACTIVE", target_card};
        }
        return {};
    });
}
```

`on_apply(game_ptr game)` is called **exactly once, when the match starts**, for every expansion selected in `game_options`: this is the place to register "global" listeners (not tied to a specific card, `nullptr` as the first argument of `add_listener`) that remain active for the entire duration of the match.

`ruleset_vtable` also exposes a second, optional hook, `is_valid_with(const expansion_set&)`, used to declare **dependencies or incompatibilities between expansions**. Real example (`effects/legends/ruleset.cpp`):

```cpp
bool ruleset_legends::is_valid_with(const expansion_set &set) {
    return set.contains(GET_RULESET(legends_basemod));
}
```

This prevents *Legends* from being activated without also activating *Legends (Base game mods)* — the combination gets rejected by `validate_expansions()` (called when the client sends `lobby_game_options`, in `game_options.cpp`). If your `struct` doesn't define `is_valid_with`, the framework falls back to a default that **always accepts** (no constraint) — see the `TRY_RETURN(...)` pattern in `vtable_build.h`.

---

## 3. Two ways to add "shared state": tokens vs a new pile

Many expansions introduce a new shared resource. The framework offers two paths, of very different invasiveness:

### 3.1 A new *token* (lightweight, no changes to the "core" needed)

If the resource is a simple **counter/token** (not a pile of cards), the generic token system (`card_token_type`) is used, which is already extensible. Real example — the "cube" tokens of *Armed & Dangerous* (`effects/armedanddangerous/ruleset.cpp`):

```cpp
void ruleset_armedanddangerous::on_apply(game_ptr game) {
    // At game setup, place 32 available cubes on the table
    game->add_listener<event_type::on_game_setup>({nullptr, 4}, [](player_ptr origin){
        origin->m_game->add_tokens(card_token_type::cube, 32, token_positions::table{});
    });
    // ... other listeners that move cubes between the table/cards/characters
}
```

This **doesn't require touching `game_table.h`**: `add_tokens`/`move_tokens`/`num_tokens` are generic APIs already available for any `card_token_type`. You only need to add the new value to the `card_token_type` enum (in `cards/card_defs.h`) if a suitable token type doesn't already exist.

### 3.2 A new *persistent pile* (invasive, touches the engine's "core")

If instead the expansion introduces **a genuine new deck/pile** with its own position on the table (like the *train* from *The Great Train Robbery*, or the *stations*), the central `pocket_type` enum in `cards/card_defs.h` needs to be extended:

```cpp
enum class pocket_type {
    ...
    stations,
    train_deck,
    train,
    feats_deck,
    feats_discard,
    feats
    // <- a new pile would be added here
};
```

along with the matching field (`card_list`) in `game_table.h`, plus the management logic (shuffling, initial setup) in the new expansion's `ruleset.cpp`. This is the only part of the guide that is **not isolated within an `effects/<expansion>/` folder**: it touches a file shared by the entire engine, so it must be done carefully (and typically also requires updating the corresponding `PocketType` on the frontend side, see §5).

**Rule of thumb**: if the new mechanic can be expressed as "a number that goes up/down, associated with cards/players/the table," use tokens (§3.1). A new pile (§3.2) is only needed if cards must physically *sit* in a new, visible, shufflable position.

---

## 4. End-to-end example: creating the fictional "Tumbleweed" expansion

Let's put it all together with a minimal but complete example: an invented expansion called **"Tumbleweed"** that introduces:
- a simple global rule (no new pile/token: when a player ends their turn with an empty hand, they draw 1 extra card at the start of the next turn);
- a card reusing already-existing effects (no C++ for the card itself, as in Case 1/2 of the previous guide).

### 4.1 The cards' YAML file (`src/config/sets/tumbleweed.yml`)

```yaml
main_deck:
  - name: TUMBLEWEED_GUST
    signs:
      - 7 hearts
    image: tw_gust
    color: brown
    effects:
      - draw(1)
    tags:
      - strong(1)
```

### 4.2 Registration in the index (`src/config/bang_cards.yml`)

```yaml
tumbleweed: !include sets/tumbleweed.yml
```

### 4.3 The C++ ruleset (`src/effects/tumbleweed/ruleset.h` + `.cpp`)

```cpp
// ruleset.h
#ifndef __TUMBLEWEED_RULESET_H__
#define __TUMBLEWEED_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {
    struct ruleset_tumbleweed {
        void on_apply(game_ptr game);
    };
    DEFINE_RULESET(tumbleweed, ruleset_tumbleweed)
}

#endif
```

```cpp
// ruleset.cpp
#include "ruleset.h"
#include "game/game_table.h"

namespace banggame {
    void ruleset_tumbleweed::on_apply(game_ptr game) {
        game->add_listener<event_type::on_turn_end>({nullptr, 10}, [](player_ptr origin, bool skipped) {
            if (origin->m_hand.empty()) {
                origin->m_game->add_log("LOG_TUMBLEWEED_BONUS_DRAW", origin);
                origin->draw_card(1);
            }
        });
    }
}
```

(In a real implementation, you'd want to check for the most appropriate event — e.g. distinguishing "end of turn" from "start of the next turn" — but for the purposes of this example, what matters is the *pattern*: a global listener registered in `on_apply`, no new pile or token.)

### 4.4 Collector header and expansion folder

`src/effects/tumbleweed/effects.h` (analogous to the ones in other expansions, even though here there's only the ruleset):
```cpp
#ifndef __TUMBLEWEED_EFFECTS_H__
#define __TUMBLEWEED_EFFECTS_H__

#include "ruleset.h"

#endif
```

`src/effects/tumbleweed/CMakeLists.txt`:
```cmake
target_sources(bangserver PRIVATE
    ruleset.cpp
)
```

### 4.5 Wiring the new folder into the rest of the build

In `src/effects/CMakeLists.txt`, add:
```cmake
add_subdirectory(tumbleweed)
```

In `src/effects/effects.h`, add:
```cpp
#include "tumbleweed/effects.h"
```

(If the expansion also introduced a new target type, `target_types/tumbleweed/` would similarly need to be created and referenced in `target_types/target_types.h` — see Case 6 of the previous guide.)

### 4.6 On the frontend side: making the expansion selectable

**`bangweb/src/Scenes/Game/Model/CardEnums.ts`** — add the string to the union:
```ts
export type ExpansionType =
    'ghost_cards' |
    'dodgecity' |
    ...
    'frontier' |
    'tumbleweed';   // <- new entry
```

**`bangweb/src/Scenes/Lobby/GameOptionsEditor.tsx`** — add a checkbox in the appropriate group (here, "main expansions"):
```tsx
<ExpansionCheckbox name='tumbleweed' />
```
If the expansion depended on another one, or was incompatible with it (like `is_valid_with` on the server), this is expressed via `onSelect`/`onDeselect`, e.g.:
```tsx
<ExpansionCheckbox name='tumbleweed' onSelect={e => e.add('ghost_cards')} />
```

**In each folder under `bangweb/src/Locale/<Language>/`** (Italian, English, Spanish, Czech, Hungarian):
- `Labels.ts`, `ExpansionType` group: add `tumbleweed: "Tumbleweed"` (or a translation).
- `Cards.tsx`: add the entry for `TUMBLEWEED_GUST` with localized name and description.
- `GameStrings.tsx`: if the ruleset generates a new `format_str` (here `LOG_TUMBLEWEED_BONUS_DRAW`), add the corresponding entry describing how to render it in JSX.

**Graphic assets**: the `tw_gust` image referenced in the YAML must be added to `bangweb/public/cards/` (in the format/path used by other cards in the same deck).

---

## 5. Summary checklist

| Step | Files involved | Always required? |
|---|---|---|
| Define the expansion's cards | `config/sets/<expansion>.yml` | Only if the expansion introduces physical cards |
| Register the expansion in the index | `config/bang_cards.yml` | **Yes, always** (even with `{}` if there are no cards) |
| Implement global rules | `effects/<expansion>/ruleset.h/.cpp` with `DEFINE_RULESET` | Yes, unless the expansion is *purely* a list of cards reusing existing effects with no cross-cutting rule |
| Declare compatibility/dependencies | `is_valid_with` in the ruleset | Only if the expansion requires or excludes other expansions |
| New expansion-specific effects/targets | `effects/<expansion>/*.cpp`, `target_types/<expansion>/*.cpp` | Only if the cards can't be expressed with existing building blocks (see previous guide) |
| New shared resource (lightweight) | `card_token_type` (`cards/card_defs.h`) + `add_tokens`/`move_tokens` in the ruleset | Only if a new global "counter" is needed |
| New persistent pile (heavyweight) | `pocket_type` + a field in `game_table.h` | Only if cards must physically occupy a new game area |
| Wire the folder into the build | `effects/CMakeLists.txt` (+ `add_subdirectory`), `effects/effects.h` (+ `#include`), and the same for `target_types/` if needed | **Yes, always**, if new `.cpp` files are added |
| Make the expansion selectable | `bangweb/.../CardEnums.ts` (`ExpansionType`), `GameOptionsEditor.tsx` (`<ExpansionCheckbox>`) | **Yes, always** |
| Translate the expansion name | `Locale/<every language>/Labels.ts`, `ExpansionType` group | **Yes, always**, for every supported language |
| Translate the new cards | `Locale/<every language>/Cards.tsx` | Yes, for every new card |
| Translate new log/prompt messages | `Locale/<every language>/GameStrings.tsx` | Only if the ruleset/cards generate new `format_str`s |
| Graphic/audio assets | `bangweb/public/cards/`, `public/sounds/`, an optional `sounds:` section in the YAML | If the card has a dedicated illustration or sound |

---

## 6. Note: "rule-only" expansions with no cards of their own

Not every "expansion" in the game's sense introduces new physical cards — some are **rule variants** layered on top of the base deck. In the project, `ghost_cards`, `stickofdynamite`, and `shadowgunslingers` are mapped to `{}` in the `bang_cards.yml` index: they have no card YAML file, but they still exist as a registered `ruleset_vtable` (`DEFINE_RULESET`) and as a selectable entry in the lobby (typically in the "Variants" group of the options editor, not "Main expansions" — see the distinction between the `Collapsible` "expansions"/"variations"/"extras" groups in `GameOptionsEditor.tsx`). This is the right pattern to follow when you want to add **a rule variant** (e.g. "optional rule X") rather than an actual card package: you write only the `ruleset.cpp`, register `variantname: {}` in the index, and add the frontend checkbox in the appropriate group.

---

*Document generated by analyzing the state of the `master` branch of the `bang-server` and `bangweb` repositories as of July 9, 2026.*
