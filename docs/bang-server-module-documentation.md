# Module documentation — bang-server

Repository: [bangsalvoserver/bang-server](https://github.com/bangsalvoserver/bang-server)
Language: **C++23** (backend) + **Python 3** (build-time code generation)
Size: ~33,500 lines across `.h`/`.cpp`/`.py`, 302 `.cpp` files, 376 `.h` files

This document describes in detail the internal architecture of the **Bang!** game server (bang.salvoserver.it), analyzing the structure under `src/` module by module.

---

## 1. Architecture overview

The source code is split into **6 top-level modules**, each with its own `CMakeLists.txt`:

```
src/
├── net/            Networking, sessions, lobby, message protocol
├── game/            Game engine (state, turns, requests, general rules)
├── cards/            Static "vtable" system defining card effects
├── effects/          Effect implementations, organized by expansion
├── target_types/     Target-type implementations, organized by expansion
├── config/            YAML files describing cards + Python scripts generating C++
└── utils/            Generic utility library (independent of the "Bang!" domain)
```

The key idea behind the whole project is a **clean separation between data and engine**:

- **Cards** (name, suits, effects, targets, tags...) are declaratively described in **YAML** files (`src/config/sets/*.yml`).
- A Python script (`src/config/parse_bang_cards.py`) reads these YAML files and generates a **C++ source file** (`bang_cards.cpp`) that populates static data tables (`card_data`).
- Every "type" of effect/target/rule referenced in the YAML (e.g. `bang`, `beer`, `player`, `weapon(3)`) must have a corresponding C++ implementation registered via macros (`DEFINE_EFFECT`, `DEFINE_TARGETING`, etc.) in `src/effects/` and `src/target_types/`.
- At runtime, the game engine (`game/`) interprets these tables to determine which cards are playable, which targets are valid, and what happens when a card is played — all **without `virtual`**, using function pointers in static "vtable" structs (manual, zero-cost polymorphism).

This pattern makes it very easy to add new expansions: you write the card's YAML, and if a new effect is needed, you implement a small C++ struct with a few static functions.

---

## 2. Module `net/` — Networking and lobby management

Handles everything **external to a single game engine instance**: WebSocket connections, user sessions, lobbies, the client/server message protocol, profile pictures, statistics tracking.

| File | Responsibility |
|---|---|
| `main.cpp` | Process entry point. CLI argument parsing (`cxxopts`), logging setup, signal handling (`SIGINT`/`SIGTERM`), main fixed-tick loop (`server.tick()` at `ticks64{1}` intervals), server startup (`server.start(port, reuse_addr)`). |
| `manager.h` / `manager.cpp` | The `game_manager` class, the heart of the network layer. Inherits from `net::wsserver` and manages the entire lifecycle: connections (`on_connect`/`on_disconnect`/`on_message`), dispatch of client messages (`handle_message` overload for each `client_messages::*` type), session management (`m_sessions`), lobbies (`m_lobbies`), and active connections (`m_connections`). |
| `wsserver.h` / `wsserver.cpp` | Wrapper around the **uWebSockets**-based WebSocket server; exposes the abstract interface (`on_connect`, `on_disconnect`, `on_message`) that `game_manager` implements. Also handles optional TLS/SSL support (`init_tls`). |
| `lobby.h` / `lobby.cpp` | Lobby data structures: `game_lobby` (a game room with users, bots, chat, options, state), `game_session` (a persistent user session with username/avatar), `game_user` (a user within a specific lobby, with flags like `spectator`, `muted`, `lobby_owner`), `lobby_bot`. Also includes `command_permissions`/`lobby_command` for chat commands. |
| `lobby_commands.cpp` | Implementation of the text commands typed in the lobby chat (e.g. lobby-owner commands, cheats, etc.), registered via `add_command`. |
| `messages.h` / `messages.cpp` | Defines the entire client↔server **communication protocol** as a `std::variant` of strongly-typed structs: `client_message` (e.g. `connect`, `lobby_make`, `lobby_join`, `game_action`, `game_rejoin`...) and `server_message` (e.g. `lobby_update`, `lobby_chat`, `game_update`, `game_started`...). Provides `serialize_message`/`deserialize_message` for JSON conversion. |
| `game_interface.h` | An abstract interface (`game_interface`) that decouples the network layer from the actual game engine (implemented in `game/game.h`), following a two-tier pattern: `net/` knows nothing about game internals, only this interface. |
| `bot_info.h` | Configuration data structure for bot behavior (read from `config/bot_info.yml`): heuristic rules for the bots' weighted-random decisions. |
| `image_pixels.h` / `.cpp` | Representation of raw images (user avatars) as pixel buffers, with hashing for deduplication. |
| `image_registry.h` / `.cpp` | Global registry of images (avatars/profile pics) with reference counting (`registered_image`, RAII), to avoid duplicating identical images shared by multiple users in memory; uses `libpng` for (de)serialization. |
| `logging.h` / `.cpp` | Logging wrapper with levels (`trace`, `debug`, `info`, `warn`, `error`) configurable from the CLI. |
| `tracking.h` / `.cpp` | Persistence of server statistics (player/lobby counts over time) in **SQLite** (optional, via `--tracking-db`). |
| `options.h` / `server_options.h` | Server configuration options: default port, `enable_cheats` flag, `disable_pings`, etc. |

### Typical connection flow
1. The client opens a WebSocket connection → `wsserver` invokes `on_connect` → `game_manager` creates a `connection` in `not_validated` state.
2. The client sends `client_messages::connect` (with `username`, `propic`, `session_id`) → the manager validates/creates the `game_session`, moving state to `connected`.
3. The user creates (`lobby_make`) or joins (`lobby_join`) a `game_lobby`.
4. When the game starts (`game_start`), the lobby instantiates a `banggame::game` (through `game_interface`), and subsequent messages (`game_action`) are forwarded to the game engine.
5. Periodic ping/pong and inactivity timers keep the connection alive/valid (`ping_timer`, `inactivity_timer` in `connection_state::connected`).

---

## 3. Module `game/` — Game engine

The logical heart of the server: manages the state of a single match, the turn, pending requests, players, cards on the table, events, and general rules independent of specific expansions.

### 3.1 Core data structures

- **`game_table`** (`game_table.h/.cpp`) — The "state of the world" for a match. Multiply inherits from `game_net_manager`, `listener_map`, `disabler_map`, `request_queue` (architectural mixins). Contains:
  - All card decks/piles as `card_list`: `m_deck`, `m_discards`, `m_selection`, `m_shop_deck`, `m_scenario_deck`, `m_stations`, `m_train`, `m_feats`, etc. (later expansions introduce new piles: shop, train stations, feats...).
  - `m_players`: the ordered list of players at the table.
  - `rng`/`bot_rng`: two separate random generators (one for game mechanics, one for bot AI, so bot behavior is deterministic/reproducible independently of the deck).
  - `m_first_player`/`m_playing`: who started the match and whose turn it currently is.
  - Support methods: `calc_distance` (distance calculation between players for weapon range), `range_alive_players`/`range_other_players` (C++20 range-views to iterate over alive players starting from a given one), `add_cards_to`/`remove_cards`, `shuffle_cards_and_ids`.

- **`game`** (`game.h`) — A concrete subclass of `game_table` that implements the `game_interface` exposed to the `net/` module: `handle_game_action`, `get_pending_updates`, `rejoin_user`, `start_game`, `request_bot_play`, etc. It's the sole point of contact between network and engine.

- **`player`** (`player.h/.cpp`) — Represents a single player: hand (`m_hand`), cards on the table (`m_table`), characters (`m_characters`), role (`sheriff`/`deputy`/`outlaw`/`renegade`, including "shadow" variants for special modes), HP (`m_hp`/`m_max_hp`), tokens (bullets, cubes, gold...). Exposes the fundamental operations: `draw_card`, `discard_card`, `steal_card`, `equip_card`, `damage`/`heal`, `pass_turn`, computation of `get_range_mod`/`get_weapon_range`/`get_distance_mod` for table geometry.

- **`card`** (`card.h/.cpp`) — A single card instance on the table: reference to static data (`card_data`, generated from YAML), dynamic state (owner, visibility, pocket/position).

- **`card_data`** (`cards/card_data.h`) — Immutable data generated at compile time from YAML: name, image, suit/rank (`card_sign`), color, lists of `effect_holder`/`equip_holder`, tags.

### 3.2 Event and "disabler" system (the engine behind card interactions)

- **`event_map.h`** — Implements a **typed event bus** based on compile-time reflection (`reflect::to<std::tuple>`), with no inheritance from a common event base class: any aggregate struct can be used as an "event" (`concept event`) — declaring the struct **is** the registration, no macro needed, unlike the `effects`/`target_types` vtable system. Two separate maps back this, each with a distinct job: `m_listeners` (keyed by `event_listener_key`) actually holds the listeners and dictates dispatch order — **descending priority** first, then ascending `card->order` (table/play order) as a deterministic tiebreak between equal priorities. Real `on_hit` listeners alone span priorities from 1 (*Bart Cassidy*) up to 20 (a global ruleset rule), precisely because each needed to fire in a specific position relative to the others. `m_map` (keyed by the plain `event_card_key`) holds only iterators *into* `m_listeners` and exists purely to support fast removal of a card's listeners when it leaves play — it plays no part in dispatch order. `listener_map::call_event<T>(value)` invokes listeners in that priority order; if `T` declares a `result_type`, the first listener whose return value is truthy stops the chain and becomes the event's result (e.g. `check_damage_response`'s `bool`, used to decide whether to delay resolving damage); without one, the event is void and every listener runs unconditionally as a pure broadcast.
- **`disabler_map.h`** — A separate mechanism for **disabling cards** (e.g. *Sid Ketchum* disabled while "paused", cards disabled by other cards like *Handcuffs*), via predicate functions registered per `event_card_key` (`card_disabler_fun`).
- **`event_card_key.h`** — The `{target_card, priority}` pair passed to `add_listener`/`add_disabler`, identifying which card a listener belongs to (for later removal via `m_map`) and its priority for the dispatch-order tiebreaking described above.

### 3.3 Request cycle (turns, prompts, pending actions)

- **`request_base.h` / `request_queue.h/.cpp`** — The game proceeds via a **priority request queue** (`m_requests`, a `utils::stable_priority_queue`). Each `request_base` represents a pending action (e.g. "player X must respond with Missed!", "resolving a Duel"), and `request_queue` tracks one overall `request_state` — `done`, `next`, `waiting{timer}`, or `bot_play{timer}` — describing what should happen next. The mechanism is split into two distinct halves, not a single per-tick step:
  - **`tick()`**, called once per server tick, only drives `invoke_tick_update()` — the *waiting* half. If the top request has an enabled `request_timer`, it counts that down (finishing it once expired); otherwise it just counts down whatever the current `waiting{timer}`/`bot_play{timer}` state is, transitioning to `next` once a `waiting` timer reaches zero, or actually invoking `request_bot_play(true)` once a `bot_play` timer reaches zero. If this resolves to `next`, `tick()` calls `commit_updates()`.
  - **`commit_updates()`** is the half that actually advances gameplay: it calls `invoke_update()` — which calls the top request's own `on_update()`, and, if the request itself didn't change, checks/restarts its timer and calls `request_bot_play(false)` — in a tight loop, for as long as the result keeps coming back `next`. This lets a whole cascade of automatically-resolving requests (e.g. one card's effect immediately queuing another) play out synchronously within a single tick, bounded by two safety limits (`max_update_count = 30`, `max_update_timer_duration = 10s`) that force a `waiting` state instead of looping indefinitely.
  - `request_bot_play` sits at the end of `invoke_update()` almost unconditionally (not just when a bot's turn is detected up front) — it's the single place that decides, after every step, whether the next thing to do is hand control to a bot (scheduling `bot_play{timer}`) or simply wait for a human response (`done`).
- **`request_timer.h`** — Requests with a timeout (e.g. a time limit to respond to a Missed!), counted down by `invoke_tick_update()` above.
- **`durations.h/.cpp`** — Manages animation/timer durations, with a configurable `duration_coefficient` (slows down/speeds up the pace perceived on the client).

### 3.4 Play verification and resolution

- **`play_verify.h/.cpp`** — The central **validation** point for an action proposed by the client (`game_action`): checks whether the card is playable, whether the targets are valid according to the filters (`filters.h`), whether any modifier cards precede it are compatible (e.g. *Aim*, which must be followed by a Bang! — see §4 for the full modifier mechanism), producing a result among `ok`, `prompt` (requires confirmation or a further choice), and `error` (with a localized message).
- **`possible_to_play.h/.cpp`** — Generates the set of cards/targets **actually playable** by a player at a given moment (used both to send the client the list of available actions, and by the bot AI to generate plausible random moves — `generate_playable_cards_list`).
- **`filters.h/.cpp`** — Implements `check_player_filter`/`check_card_filter`, i.e. validation of the "filters" declared in YAML (e.g. `alive`, `notself`, `reachable`, `range_1`) against a candidate target.
- **`give_card.h/.cpp`** — Implementation of the `/give <card_name>` cheat command, available when the server is started with `--cheats`: looks up a card by name and hands it to a player, useful for manually testing a specific card without waiting to draw it.
- **`prompts.h/.cpp`** — A shared library of reusable prompt/validation-message constructors that individual card effects call into from their own `get_error`/`on_prompt`, instead of each duplicating the same checks. Two families: ordinary human-facing ones (`prompt_target_self`, `prompt_target_ghost`, `prompt_target_self_card`, `prompt_target_immunity`) that warn or block regardless of who's playing, and a `bot_check_*` family (`bot_check_kill_sheriff`, `bot_check_target_enemy`, `bot_check_target_friend`, `bot_check_target_card`, `bot_check_discard_card`) that each start with an `origin->is_bot()` guard, so they're silently invisible to human players and only ever steer bot decisions (see the card-creation guide's bot-prompts case for exactly how that steering works). The file also provides the two combinators used to reconcile several simultaneously-applicable prompts on the same card into one: `select_prompt` (picks the highest-`priority` non-empty one) and `select_prompt_fallback_empty` (the same, but suppresses a merely-default-priority result if any contributing slot explicitly wanted no prompt at all).

### 3.5 Bot / AI

- **`bot_ai.cpp`** — Implements `game::request_bot_play`: when it's a bot's turn (or it must respond to a request), it generates a weighted random play (`generate_random_play`) from the list of playable cards, applies heuristic rules (`bot_rules.h/.cpp`, e.g. "prefer healing if low on HP") via an ordered multiset of "card nodes" (`card_node`), attempts the play with `verify_and_play`, and retries with increasing attempts up to a limit (`max_random_tries`) before declaring a stalemate ("softlock").
- **`bot_rules.h/.cpp`** — Definition of the rules/heuristics used to weight bot choices, separately for the play phase (`in_play_rules`) and the response phase (`response_rules`).
- **`bot_suggestion.h/.cpp`** — The heuristics bots use to guess *who's on their side* in a hidden-role game where a player's actual role often isn't known. It tracks a lightweight, mutually-exclusive "karma" flag (`positive`/`negative`/neutral) per unknown player, nudged by `signal_hostile_action`/`signal_helpful_action` every time a bot observes someone attack or help someone else (`signal_remove_card` is a convenience wrapper that treats destroying a card as hostile, unless that card is a `penalty`-tagged one like Jail — removing that is actually helpful): when the *target* of an observed action has a known role, the *acting* player's karma is set directly from simple Bang! alignment rules (attacking a known lawman reads as bandit-like, attacking a known outlaw/renegade reads as lawman-like); when the target's role is itself still unknown, karma propagates transitively through a small fixed lookup table instead. `is_target_enemy`/`is_target_friend` then combine a bot's own (always-known) role, whatever's actually been revealed, and this accumulated karma into a real verdict, implementing Bang!'s full alignment logic per role — including the Renegade's context-dependent flip (siding with whichever faction, law or outlaws, currently has fewer members left) and the 3-player and free-for-all variants. These two functions are the actual reasoning behind every `bot_check_target_friend`/`bot_check_target_enemy` call in `prompts.cpp`.

### 3.6 Other support elements

- **`expansion_set.h/.cpp`** — Bitset representation (`uint64_t`) of the set of expansions active in a match (`expansion_set`), with iterators, JSON (de)serialization, and string parsing (e.g. for CLI/config options).
- **`game_options.h/.cpp`** — Match options configurable from the lobby: active expansions, max player count, various timers (`auto_resolve_timer`, `damage_timer`, `escape_timer`, `bot_play_timer`), flags like `add_bots`, `only_base_characters`, RNG seed.
- **`game_update.h`** — Defines the "state update" (`game_update`) messages sent to the client to reflect every change (e.g. card drawn, damage taken, turn passed); these are JSON-serialized and queued via `game_net_manager::add_update`.
- **`game_net.h/.cpp`** (`game_net_manager`) — Manages buffering of updates to send to clients (`m_updates`), the persistent game log for rejoins (`m_saved_log`), and the mapping between internal `player_ptr`s and network `user_id`s (to handle disconnections/reconnections).
- **`event_card_key.h`**, **`card.h`** — keys/identifiers used across the event system.

---

## 4. Module `cards/` — The static "vtable" system

This is the most conceptually original module of the project: it defines **how an effect/target/rule is represented** so that it can be auto-generated by the YAML parser and resolved at runtime **without the overhead of virtual inheritance**.

| File | Contents |
|---|---|
| `card_defs.h` | Fundamental data structures: `card_sign` (suit+rank), `card_color_type`, `card_token_type` (bullets, cubes, gold, fame...), `player_role`, `pocket_type` (the possible "positions" of a card: hand, table, deck, discard, shop, train, feats...), `effect_holder`/`equip_holder`/`modifier_holder`/`mth_holder` (the type-erased polymorphic "wrappers"), `play_card_target` (an `std::any`-like wrapper for a generic target). |
| `vtables.h` | The actual **vtables**: structs such as `effect_vtable`, `equip_vtable`, `modifier_vtable`, `mth_vtable`, `ruleset_vtable`, `targeting_vtable` — each a bundle of **function pointers** (`can_play`, `get_error`, `on_prompt`, `add_context`, `on_play`, etc.) plus `effect_value` (a type-erased pointer to the effect's specific data, e.g. the numeric value of a `weapon(3)`). The macros `DEFINE_EFFECT(NAME, Type)`, `DEFINE_EQUIP`, `DEFINE_MODIFIER`, `DEFINE_MTH`, `DEFINE_RULESET`, `DEFINE_TARGETING` register a template specialization (`effect_vtable_map<"name">`) linking the **string used in YAML** (e.g. `"bang"`, `"weapon"`, `"player"`) to the concrete C++ type implementing it — but by themselves they only *declare* that link (`static const effect_vtable value;`, with no definition); see below for where the definition actually gets built. The macros `GET_EFFECT(name)`/`TARGET_TYPE(name)`/etc. resolve the pointer to the corresponding vtable at compile time. |
| `vtable_build.h` | The template machinery that actually *builds* a vtable from a concrete C++ type, plus a build-time switch that controls where that construction gets emitted. For each kind (`effect`, `equip`, `modifier`, `mth`, `ruleset`, `targeting`) it defines a `build_*_vtable<T>(name)` function that uses `requires`-based overload probing (the `TRY_RETURN` macro) to detect which of several possible member-function signatures a given type `T` implements (e.g. `can_play(origin_card, origin, ctx)` vs. `can_play(origin_card, origin)` vs. none at all, defaulting to `true`), and wraps whichever one exists into the vtable's uniform, type-erased function pointers. See the explanation below for how this gets wired to a single translation unit. |
| `card_effect.h` | The `target_args` namespace with the standard target arguments (`empty`, `player` with `player_filter_bitset`, `card` with `player_filter_bitset`+`card_filter_bitset`); the `event_equip` struct for standard handling of the equipment-disable event. |
| `effect_context.h` | `effect_context`: a heterogeneous container (a "bag" of contexts, see `contexts::*` in `card_defs.h`) that accumulates additional information while an effect is being built/played (e.g. which card is being played, which target was selected in a sub-step, whether it's a "repeat" or a forced play). Used to communicate state between chained effects (e.g. a modifier + the base effect) and to serialize/deserialize player choices. |
| `filter_enums.h` | Enumerations of the available player/card target filters (e.g. `alive`, `notself`, `reachable`, `range_1`) used as bitsets (`player_filter_bitset`, `card_filter_bitset`). |
| `game_enums.h` | General domain enumerations (e.g. `resolve_type`, `play_as`) referenced both by C++ code and by strings in the YAML (e.g. `resolve(resolve_type::resolve)`). |
| `game_events.h` | Declaration of the domain event types (the "event" structs consumed by `listener_map::call_event`), e.g. damage, draw, death, end-of-turn events. |
| `game_string.h` | `game_string`/`prompt_string`: localizable game strings with typed arguments (for error messages, prompts, logs — conceptually similar to `std::format` but serializable and translated client-side). |
| `card_serial.h` | JSON (de)serialization of cards for the network protocol (a card's client-side representation). |
| `bang_cards.h` | Header declaring the static `bang_cards_t bang_cards` tables (populated by the generated `bang_cards.cpp` file), containing all decks (`deck`, `characters`, `goldrush`, `highnoon`, `stations`, `train`, `legends`, `feats`, `button_row`, `hidden`...) as arrays of `card_data`. |
| `card_fwd.h` | Forward declarations of shared types (`card_ptr`, `player_ptr`, `ruleset_ptr`, etc.) to reduce include dependencies. |

### The two-phase macro trick behind `vtable_build.h`

`DEFINE_EFFECT(bang, effect_bang)` (and its five siblings) is invoked once per card effect, inside that effect's own header — e.g. `effects/base/bang.h`. That header is `#include`d, directly or transitively, from dozens of different `.cpp` translation units across the codebase (any file that needs to reference the `bang` effect at all). A specialization like `effect_vtable_map<"bang">::value` is a single `static const` data member, though, and C++ only allows **one** definition of it across the entire linked program — so whatever expands `DEFINE_EFFECT(bang, ...)` into an actual initializer can only be allowed to fire in exactly one of those many translation units, not all of them.

The codebase solves this with a macro that's deliberately defined twice. In `vtables.h`, right where `DEFINE_EFFECT` is defined, `BUILD_EFFECT_VTABLE(name, type)` is first given an **empty** body:
```cpp
#define BUILD_EFFECT_VTABLE(name, type)
#define DEFINE_EFFECT(NAME, TYPE) \
    template<> struct effect_vtable_map<#NAME> { using type = TYPE; static const effect_vtable value; }; \
    BUILD_EFFECT_VTABLE(NAME, TYPE)
```
So ordinarily, `DEFINE_EFFECT` only *declares* the specialization (the `static const effect_vtable value;` member has no initializer here) — harmless to repeat across many translation units, since a declaration isn't a definition. `vtable_build.h`, however, forcibly redefines the same macro to something that actually builds and initializes it:
```cpp
#ifdef BUILD_EFFECT_VTABLE
#undef BUILD_EFFECT_VTABLE
#endif
#define BUILD_EFFECT_VTABLE(name, type) template<> const effect_vtable effect_vtable_map<#name>::value = build_effect_vtable<type>(#name);
```
Because C preprocessor macros expand using whatever definition is current *at the point of use*, any `DEFINE_EFFECT(...)` invocation that happens **after** `vtable_build.h` has been included in that translation unit will now also emit the real, program-wide definition — while the very same macro invocation, unchanged, in every *other* translation unit (where `vtable_build.h` was never included) still expands to just the harmless declaration.

This is only made to happen in exactly one place: the Python generator's `INCLUDE_FILENAMES` list (`config/parse_bang_cards.py`) fixes the include order at the top of the generated `bang_cards.cpp`:
```cpp
#include "cards/bang_cards.h"
#include "cards/vtable_build.h"     // (2) flips BUILD_*_VTABLE to the "real" version
#include "effects/effects.h"        // (3) transitively re-includes every DEFINE_EFFECT/DEFINE_EQUIP/...
#include "target_types/target_types.h"
```
`effects/effects.h` and `target_types/target_types.h` are the actual **collector headers** — they `#include` every single per-expansion `effects.h`, which in turn `#include`s every individual card header (`bang.h`, `beer.h`, ...). Because `vtable_build.h` is included *before* them, every `DEFINE_EFFECT`/`DEFINE_EQUIP`/`DEFINE_MODIFIER`/`DEFINE_MTH`/`DEFINE_RULESET`/`DEFINE_TARGETING` encountered while parsing those headers — i.e. literally every card effect, equip, modifier, ruleset, and target type in the entire project — gets its vtable actually *built* right there, in this one generated translation unit. Everywhere else in the codebase, the exact same headers only ever produce lightweight declarations, resolved at link time against the single definition living in `bang_cards.cpp`'s object file. The payoff is that the (fairly heavy) templated `build_*_vtable<T>` instantiation and the resulting static data exist exactly once in the whole build, no matter how many of the ~300 `.cpp` files include a given card's header.

### How it all fits together (a concrete example)

Take the BANG! card defined in YAML (`config/sets/base.yml`):
```yaml
effects:
  - banglimit
  - bangcard player alive reachable notself
```

The Python generator (`parse_bang_cards.py`) translates this line into a C++ entry like:
```cpp
effect_holder{
    type = GET_EFFECT(bangcard),
    effect_value = EFFECT_VALUE(bangcard)(...),
    target = TARGET_TYPE(player),
    target_value = TARGET_VALUE(player)({ player_filter = {alive, reachable, notself} })
}
```
`GET_EFFECT(bangcard)` resolves, at compile time, to the pointer to `effect_vtable_map<"bangcard">::value`, whose concrete implementation lives in `src/effects/base/bang.cpp` (a struct registered with `DEFINE_EFFECT(bangcard, ...)`). `TARGET_TYPE(player)` similarly resolves the "player" target implementation in `src/target_types/base/player.cpp`. At runtime, when the engine needs to check whether the card is playable or apply its effect, it calls the vtable's function pointers, passing `effect_value`/`target_value` as context — no `virtual`, no `dynamic_cast`, everything resolved through a single indirection.

### Modifiers: a card that has to be played together with another one

Alongside `effects`/`responses`/`equip_effects`, every `card_data` carries two more, optional slots: `modifier` and `modifier_response` (each a `modifier_holder`, pointing at a `modifier_vtable`). A card with one of these set isn't a complete action by itself — the engine requires it to be immediately followed by a compatible card, and treats the two (or more) as a single combined play.

The real example is *Aim* (`effects/valleyofshadows/aim.cpp`, already used in the card-creation guide's stacking-effects case): "play this before a Bang!, and that Bang! deals +1 damage." Its YAML just says:
```yaml
- name: AIM
  modifier: bangmod
  effects:
    - aim
```
`bangmod` isn't specific to Aim — it's a generic, reusable modifier defined once in `effects/base/bang.h`, meaning "whatever comes after this must be a Bang!-like card":
```cpp
struct modifier_bangmod {
    bool valid_with_equip(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        return false;                              // never compatible with equip cards
    }
    bool valid_with_modifier(card_ptr origin_card, player_ptr origin, card_ptr target_card);  // true if target_card is also tagged bangmod (stacking)
    bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr target_card);       // true if target_card is a real Bang! (or play_as_bang)
};
DEFINE_MODIFIER(bangmod, modifier_bangmod)
```
A `modifier_vtable` only really has two operations, both visible in `card_defs.h`'s `modifier_holder`:
- **`add_context(origin_card, origin, ctx)`** — runs when the modifier is applied, letting it inject whatever it needs into the shared `effect_context` for the rest of the chain. The wrapper in `vtables.h` always also records `contexts::modifier_card{origin_card}` here, regardless of what the specific modifier does, so downstream code can always tell a modifier preceded the current play.
- **`get_error(origin_card, origin, target_card, ctx)`** — asks whether a specific candidate `target_card` is a legal thing to play next. `build_modifier_vtable` (`vtable_build.h`) doesn't call a single flat function for this: it probes the type for up to three different overloads and picks based on what the candidate actually is — `valid_with_equip` if `target_card` is an equip card, `valid_with_modifier` if `target_card` is *itself* another modifier (i.e. the player is stacking a second modifier before the real card), or `valid_with_card` for the ordinary case — falling back to a custom `get_error` override for anything bespoke that doesn't fit the three-way split.

A `game_action` sent by the client, accordingly, is never just "one card + targets": it's `{ card, targets, modifiers: [{card, effect_list, targets}, ...] }` — zero or more modifier plays, in order, followed by one terminal card. `play_verify.cpp::verify_modifiers` is what walks this chain:
1. For each modifier in order, it calls `add_context` and validates that modifier's own targets and base playability (modifiers can have targets of their own, and can be disabled/blocked like any other card).
2. It then figures out which effect-list the *terminal* card must belong to (an equip card forces `equip_effects`; if the terminal card is itself a modifier with nothing queued after it, the whole action is rejected with `ERROR_CARD_IS_MODIFIER` — a chain can never end on an unresolved modifier).
3. It re-walks the chain a second time, calling every modifier's `get_error` against the terminal card **and** against every modifier that comes after it — so with two stacked modifiers, each has to separately approve of the other *and* of the final card, not just of the end result.

Execution (`verify_and_play`) then runs the modifiers' own effects first, strictly in order (e.g. *Aim*'s `on_play`, registering the temporary `apply_bang_modifier` listener from the stacking-effects example), and only afterwards the terminal card's own effect or equip — so by the time the Bang! actually resolves, Aim's bonus is already wired into the shared request. Everything here — the chain, the pairwise compatibility, the ordering — is generic; nothing about *Bang!* or *Aim* specifically is hard-coded into `play_verify.cpp` itself.

**A second, structurally different example is Gold Rush's "choice" cards** — *Bottle* and *Pardner*, each a single physical card that can act as any one of three different effects, chosen at play time. *Bottle* plays as Panic!, Beer, or Bang! (the player's choice); *Pardner* plays as General Store, Duel, or Cat Balou. Unlike `bangmod` (which restricts the *kind* of card that follows), this modifier restricts *which hidden option* is allowed to follow — and the link between a menu card and its options is a shared tag value, not a card-type check:
```yaml
- count: 3
  name: BOTTLE
  color: brown
  modifier: card_choice

hidden:
  - name: BOTTLE_PANIC
    effects: [card_choice, steal random_if_hand_card range_1]
    tags: [card_choice(1)]
  - name: BOTTLE_BEER
    effects: [card_choice, heal]
    tags: [card_choice(1)]
  - name: BOTTLE_BANG
    effects: [card_choice, bang player alive reachable notself]
    tags: [card_choice(1)]
```
```cpp
struct modifier_card_choice {
    bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        return target_card->pocket == pocket_type::hidden_deck
            && target_card->get_tag_value(tag_type::card_choice) == origin_card->get_tag_value(tag_type::card_choice);
    }
    void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        ctx.add(contexts::card_choice{ origin_card });
    }
};
DEFINE_MODIFIER(card_choice, modifier_card_choice)
```
Every option (`BOTTLE_PANIC`/`BOTTLE_BEER`/`BOTTLE_BANG`) is itself a hidden pseudo-card whose own first effect is `card_choice` again — this time registered as an ordinary *effect*, not a modifier (`DEFINE_EFFECT(card_choice, effect_card_choice)` alongside the `DEFINE_MODIFIER`, reusing the same name for two different vtables). Its `can_play` is the other half of the same check: it looks for the `contexts::card_choice` entry left behind by `add_context`, and only allows itself to be played if its own `card_choice` tag value matches the modifier that came before it. So `tag_type::card_choice`'s *value* (1 for Bottle's three options, 2 for Pardner's) is doing the same job `tag_type::bangcard` does for `bangmod` — a shared vocabulary a modifier and its valid follow-ups both check — just grouped by an arbitrary number instead of a single fixed tag, which is what lets several unrelated hidden-card groups (Bottle's three options vs. Pardner's three) reuse the exact same modifier and effect without colliding with each other.

### Multi-target handlers (`mth`): one effect, several different targets at once

A card's `effects:`/`responses:` list can already have several entries — that's how *Sid Ketchum* combines "discard 2 cards" with "heal," each fully independent, each with its own `on_play`. That works whenever the parts genuinely don't need to know about each other. `mth` (multi-target handler) is the mechanism for the other case: when a single, coherent effect genuinely needs **several differently-filtered target groups delivered to one function at once**, rather than resolved as unrelated, independent steps.

The clean example is **Doc Holyday**: discard exactly 2 of your own hand cards, and deal 1 damage to a player of your choice — one combined action, not two unrelated ones. Its YAML:
```yaml
effects:
  - max_usages(1)
  - discard cards(2) self | hand
  - none player alive reachable notself
mth_effect: doc_holyday(1,2)
```
Every target group is still declared as an ordinary `effect_holder` entry — `discard cards(2) self | hand` is validated and filtered exactly like it would be on any other card, and `none player alive reachable notself` uses the placeholder `none` effect purely to get a validated, filtered player target with no built-in behavior of its own. `mth_effect: doc_holyday(1,2)` then says: *take slots 1 and 2 of this card's flat target list (0-indexed; slot 0 is the `max_usages` gate and has no target of its own), and hand them to `handler_doc_holyday`*:
```cpp
struct handler_doc_holyday {
    prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target);
    void on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target);
};
DEFINE_MTH(doc_holyday, handler_doc_holyday)
```
`target_cards` is the 2 discarded cards, `target` is the chosen player — as ordinary, strongly-typed C++ parameters, even though they came from two independently-filtered target groups. *Switch* (`effects/greattrainrobbery/switch_cards.h`) is a second, slightly different case — swapping a table card of yours with a table card of someone else's — showing that the two groups don't even need different C++ types to justify `mth`, only different *meanings*:
```yaml
effects:
  - none card self | table
  - none card notself | table
mth_effect: switch_cards(0,1)
```
```cpp
struct handler_switch_cards {
    game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
    prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
    void on_play(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
};
DEFINE_MTH(switch_cards, handler_switch_cards)
```
Both parameters are `card_ptr`, but the *first* one is always "your card" (filtered `self | table`) and the *second* is always "their card" (filtered `notself | table`) — a distinction the flat wire format can't express by type alone, only by position, which is exactly what the explicit `(0,1)` index list pins down.

Mechanically, `parse_mth` (`config/parse_bang_cards.py`) turns `doc_holyday(1,2)` into an `mth_holder` whose `effect_value` is an `mth_value<handler_doc_holyday>{ handler = {}, indices = {1, 2} }` (`cards/vtable_build.h`). The interesting part is how the two typed parameters (`const card_list&`, `player_ptr`) get pulled back out of the flat, type-erased `targets_view` at runtime: `mth_unwrapper` is a partial specialization keyed on the *exact member-function-pointer type* of the handler's own `on_play`/`get_error`/`on_prompt` — template argument deduction reads off `Args...` directly from whichever overload the handler happens to define, and `build_mth_args<Args...>(targets, indices, ctx)` then walks `indices` extracting `targets[indices[i]].get<Args_i>()` one by one, throwing a `game_error` if a position is out of range or holds the wrong type (with an automatic, optional trailing `const effect_context&` parameter, the same convention used everywhere else in the vtable system). None of `get_error`/`on_prompt`/`on_play` are mandatory on a handler — each is individually probed with `requires`, exactly like the rest of the vtable-building machinery.

That resolved handler is consulted in three places, all in `game/`:
- **`play_verify.cpp::verify_target_list`** — after every individual slot's own `get_error`/`can_play` has already passed, it *additionally* calls `mth.get_error(origin_card, origin, targets, ctx)`, letting the handler reject combinations that are individually fine but jointly not (e.g. a constraint spanning both groups that no single slot's filter could express).
- **`play_verify.cpp::apply_target_list`** — after every slot's own `on_play` has run (for Doc Holyday: the cards are actually discarded, `max_usages` is ticked, and the `none` slot does nothing), `mth.on_play(origin_card, origin, targets, ctx)` runs *afterwards*, receiving the same full target list and applying the part of the behavior that genuinely spans both groups (dealing the damage, in this case).
- **`possible_to_play.cpp::check_recurse`** — this is the important one for the client. `possible_to_play` enumerates every valid combination of targets for a card via recursive backtracking, one slot at a time; once a full combination for all slots has been assembled, it calls `mth.get_error(...)` on that specific combination before accepting it, backtracking otherwise. So candidate combinations that would fail the handler's joint logic are pruned out **before** they're ever offered to the client as a possibility.

That last point is why nothing about `mth` needs to leak out to the frontend at all — see the note in the frontend doc.

---

## 5. Module `effects/` — Per-expansion effect implementation

Contains the concrete implementation of **every single effect/equipment/expansion rule** referenced in the YAML files. It is organized into **15 subfolders**, one per expansion (each with its own `CMakeLists.txt` and a collector `effects.h` header):

| Folder | Expansion | Representative content |
|---|---|---|
| `base/` | Base game | Core cards: `bang.cpp`, `beer.cpp`, `missed.cpp`, `duel.cpp`, `dynamite.cpp`, `jail.cpp`, `mustang.cpp`, `scope.cpp`, base characters (`bart_cassidy`, `black_jack`, `kit_carlson`, `lucky_duke`, `slab_the_killer`...), general mechanics (`damage.cpp`, `death.cpp`, `draw.cpp`, `draw_check.cpp`, `heal.cpp`, `equip.cpp`, `steal_destroy.cpp`, `requests.cpp`, `resolve.cpp`, `max_usages.cpp`, `card_choice.cpp`). |
| `dodgecity/` | *Dodge City* | New mechanics introduced by this expansion (reusable "green" cards, new characters). |
| `fistfulofcards/` | *A Fistful of Cards* | Event cards and their rules. |
| `goldrush/` | *Gold Rush* | Additional characters. |
| `highnoon/` | *High Noon* | Specific event cards. |
| `wildwestshow/` | *Wild West Show* | Show characters, "rotating" turn management (`changewws.cpp`), specific tokens. |
| `armedanddangerous/` | *Armed and Dangerous* | New weapons, `cube` (cube tokens — see also `add_cube.cpp`), characters like *Bloody Mary*, *Molly Stark*... |
| `valleyofshadows/` | *The Valley of Shadows* | "Ghost" mechanics (`ghost.cpp`), *Bandidos*, *Poker*, *Tornado*, "escape" token. |
| `canyondiablo/` | *Canyon Diablo* | Minor expansion with dedicated targets/effects. |
| `greattrainrobbery/` | *The Great Train Robbery* | Train mechanics (`m_train`, `m_locomotive`, `train_position`). |
| `stickofdynamite/` | *A Stick of Dynamite* | A dynamite rule variant. |
| `mostwanted/` | *Most Wanted* | New characters (*Claus the Saint*, *Emiliano*...), *Handcuffs*, *New Identity*. |
| `legends/` | *Bang! Legends* | A reimplementation of an alternative game system (legends, "feats", stations). The largest after `base/`. |
| `frontier/` | The *Frontier* expansion | Dedicated rules/cards. |
| `ghost_cards/` | Ghost card rules (dead players remaining active in certain variants). |
| `crazy_greygory/` (in `config/sets`, handled as a variant) | A "crazy" character variant. |

Each `.cpp`/`.h` file in these folders typically defines:
1. A small struct representing the effect's **data** (e.g. `struct effect_weapon { int range; };` for a weapon with parametrized range).
2. Static functions implementing the behavior (`can_play`, `on_play`, `get_error`, `on_prompt`...).
3. A `DEFINE_EFFECT(yaml_name, StructType)` macro (or `DEFINE_EQUIP`/`DEFINE_MODIFIER`/`DEFINE_MTH`/`DEFINE_RULESET`) registering the name↔type link, making it visible to `GET_EFFECT(yaml_name)` used by the YAML-generated code.

The `ruleset.h`/`ruleset.cpp` files present in many expansion folders (e.g. `armedanddangerous/ruleset.cpp`, `legends/ruleset.cpp`, `mostwanted/ruleset.cpp`, `valleyofshadows/ruleset.cpp`, `wildwestshow/ruleset.cpp`, `stickofdynamite/ruleset.cpp`) implement the `ruleset_vtable`: the logic applied **when an entire expansion is activated** in a match (e.g. preparing additional decks, adding starting tokens, validating compatibility with other expansions via `is_valid_with`).

---

## 6. Module `target_types/` — Target-type implementation

Conceptually parallel to `effects/`, but for the "who/what can be the target of an effect" part. Organized by expansion (7 subfolders: `base`, `armedanddangerous`, `canyondiablo`, `dodgecity`, `fistfulofcards`, `legends`, `valleyofshadows`).

| Key file | Role |
|---|---|
| `base/none.h` | The "empty" target (an effect with no target, e.g. drawing cards for yourself). |
| `base/player.h/.cpp` | A single-player target, with a filter (`player_filter_bitset`) applied via `filters.h`. |
| `base/players.h/.cpp` | A multi-target over all players satisfying a filter (e.g. "all alive players" for Indians!/Gatling). |
| `base/card.h/.cpp` | A single-card target (in hand or on the table) with a combined player+card filter. |
| `base/cards.h/.cpp` | A multi-card target. |
| `base/random_if_hand_card.h` | A variant: if the target is a card in hand, a random one is chosen (to respect the "you can't see others' hand cards" rule — e.g. *Panic!*/*Cat Balou*). |
| `dodgecity/card_per_player.h/.cpp` | A "one card per player" target (used by *Brawl*: discard one card, then destroy one card from every other player who still has one). |
| `dodgecity/extra_card.h` | A target with an associated extra card. |
| `fistfulofcards/max_cards.h/.cpp` | A target with a maximum number of selectable cards. |
| `armedanddangerous/cube_slot.h/.cpp`, `select_cubes*.h/.cpp`, `player_per_cube.h/.cpp`, `move_cube_slot.h/.cpp` | A family of targets dedicated to the **cube token** mechanic introduced by this expansion (slot/cube selection, movement). |
| `valleyofshadows/adjacent_players.h/.cpp` | A target restricted to adjacent players (used by *Fanning*: a Bang! against two players who are adjacent to *each other*, not to the origin). |
| `valleyofshadows/bang_or_cards.h/.cpp` | An "alternative" target (either play a Bang!, or discard cards) — a fork-choice logic. |
| `legends/missed_and_same_suit.h/.cpp` | A target with a constraint on cards of the same suit in addition to the Missed! response. |
| `canyondiablo/conditional_player.h` | A player target conditioned by an additional predicate. |

Each implementation registers, via `DEFINE_TARGETING(yaml_name, StructType)`, the corresponding `targeting_vtable`, which provides: deserialization of the target chosen by the client (`deserialize_target`), enumeration of possible targets for AI/validation (`possible_targets`, a C++23 `std::generator`), random choice (`random_target`), validation (`get_error`), confirmation messaging (`on_prompt`), and execution (`on_play`).

---

## 7. Module `config/` — Card data and code generation

This module **contains no game logic**, only the **declarative data** and the tools to turn it into C++ code at build time.

| File/folder | Role |
|---|---|
| `sets/*.yml` | One YAML file per expansion (`base.yml`, `dodgecity.yml`, `goldrush.yml`, `highnoon.yml`, `fistfulofcards.yml`, `wildwestshow.yml`, `wildwestshow_characters.yml`, `armedanddangerous.yml`, `valleyofshadows.yml`, `canyondiablo.yml`, `greattrainrobbery.yml`, `stickofdynamite.yml`, `mostwanted.yml`, `legends.yml`, `legends_basemod.yml`, `frontier.yml`, `crazy_greygory.yml`, `udolistinu.yml`). Each file lists cards split by deck (`main_deck`, `character`, `button_row`, `hidden`, `station`, `train`, `feats`, etc.), with fields such as `name`, `signs` (suit/rank), `image`, `color`, `effects`, `responses`, `equip`, `equip_effects`, `tags`, `expansion`. |
| `bang_cards.yml` | The "index" file listing which expansion YAML files to include/combine in the build (read by `parse_bang_cards.py` via a custom `!include`, see `yaml_custom.py`). |
| `bot_info.yml` | Configuration of bot AI heuristics (in-play/response preference rules, thresholds for random attempts). |
| `bot_propics/*.png` | A set of default avatar images used by bots. |
| `parse_bang_cards.py` | The main script: reads the YAML files (functions `parse_effects`, `parse_equips`, `parse_tags`, `parse_modifier`, `parse_mth`, `parse_expansions`, `parse_sign`), validates them with regular expressions (extracting effect type, parenthesized parameters, player/card filters separated by `|`), and produces a `CppObject` representing the entire data table (`bang_cards_t bang_cards`). The `merge_cards` function merges cards from all expansions into a single structure, annotating each card with its originating expansion. `parse_file` maps each logical "deck" (`Deck`) to its expansion strategy: e.g. `get_main_deck_cards` duplicates a card for every suit/rank listed in `signs`, `get_goldrush_cards` repeats it `count` times, `get_legends_cards` renames it with a `LEGEND_` prefix. |
| `cpp_generator.py` | A support library for generating C++ code from Python structures (`CppObject`, `CppEnum`, `CppLiteral`, `CppStatic`, `CppStaticMap`, `CppDeclaration`, `print_cpp_file`): abstracts "pretty-printing" of nested C++ aggregate initializers, fully-qualified enums, static maps (`static_map`), pointers to static data, etc. |
| `parse_bots.py` | An analogous parser for `bot_info.yml`, generating the C++ bot-rule table. |
| `yaml_custom.py` | Extensions to the standard YAML parser (presumably an `!include` tag for composing multiple files, given its use in `bang_cards.yml`). |

**CMake integration**: during the build, these Python scripts are invoked (presumably from `src/config/CMakeLists.txt`, as a code-generation step) to produce `bang_cards.cpp` (and the bot equivalent) **before** C++ compilation, so that the `bang_cards`/`bot_info` tables are available as native C++ symbols — no YAML parsing happens at runtime on the production server.

---

## 8. Module `utils/` — Generic utility library

Reusable components independent of the "Bang!" domain, used across all other modules:

| File | Purpose |
|---|---|
| `enum_bitset.h` | A strongly-typed bitset over an `enum class` (used everywhere for flags: `player_flags`, `game_flags`, `player_filter_bitset`...). |
| `enum_map.h` | An enum-indexed associative array (used for `token_map`: token quantities per type). |
| `enums.h` | Support functions on enums: to/from string conversion (`enums::from_string`), iteration, `is_between`. |
| `fixed_string.h` | A constant string usable as a **template parameter** (C++20 NTTP), underpinning the `effect_vtable_map<utils::fixed_string Name>` mechanism that binds YAML names to C++ types. |
| `function_ref.h` | A lightweight, non-owning reference to a function (an allocation-free alternative to `std::function`). |
| `json_serial.h` / `json_aggregate.h` | A generic JSON (de)serialization framework based on compile-time reflection (`serializer<T>`, `deserializer<T>`), used for all network messages and state updates. |
| `parse_string.h` | Generic string→type parsing (`string_parser<T>`), used e.g. for `expansion_set` and `game_options::set_option`. |
| `combinations.h` | Combination generation (likely for deck/probability calculations or for the AI). |
| `int_set.h` | An efficient set of small integers (bitset). |
| `nullable.h` | A wrapper for optional values with custom semantics. |
| `random_element.h` | Extraction of a random element from a range, with error handling (`random_element_error`, used by `bot_ai.cpp`). |
| `range_utils.h` / `ranges_concat.h` | Extensions to C++20/23 ranges (e.g. `rotate_range` used in `game_table::range_all_players`, concatenation of heterogeneous ranges). |
| `stable_queue.h` | A **stable** priority queue (preserves insertion order for equal priority) — used by `request_queue`. |
| `static_map.h` | A compile-time-built map (`static_map_view`), used for card `tag_map`s. |
| `tagged_variant.h` | A tag-dispatched variant, for handling heterogeneous type unions with minimal overhead. |
| `tsqueue.h` | A thread-safe queue (likely for passing messages between the network thread and the tick loop). |
| `type_name.h` | Compile-time type-name introspection (debug/log). |
| `visit_indexed.h` | Support for `std::visit` with the index of the active alternative. |
| `sqlite3_wrapper.h` | A minimal RAII wrapper around SQLite3, used by `net/tracking.cpp`. |
| `base64.h` | Base64 encoding/decoding (likely for profile pictures transmitted via JSON). |
| `misc.h` | Miscellaneous utilities not otherwise categorized. |

---

## 9. End-to-end flow: from YAML to network packet

To clarify how all the modules collaborate, here is the complete path of a single game action:

1. **Data definition** — The `BANG` card is described in `config/sets/base.yml` with `effects: [banglimit, "bangcard player alive reachable notself"]`.
2. **Code generation** — `parse_bang_cards.py` (using `cpp_generator.py`) translates this line into a static `effect_holder` initialization inside `bang_cards.cpp`, referencing `GET_EFFECT(bangcard)` and `TARGET_TYPE(player)`.
3. **Implementation** — `effects/base/bang.cpp` defines the struct implementing the effect (damage, animation, "Missed!" response request) and registers it with `DEFINE_EFFECT(bangcard, ...)`. `target_types/base/player.cpp` implements the player-target validation/selection logic.
4. **Runtime — proposed action** — The client sends `client_messages::game_action` (JSON) → `net::game_manager::handle_message` forwards it to `game::handle_game_action` (`game/game_net.cpp`).
5. **Verification** — `play_verify.cpp` deserializes the target (`targeting_vtable::deserialize_target`), checks the filters (`filters.cpp`) and conditions (`effect_vtable::can_play`/`get_error`).
6. **Execution** — If valid, `effect_vtable::on_play` applies the effect (e.g. queuing a `request_base` representing "the target must respond with Missed!" in the `request_queue`), possibly triggering events (`listener_map::call_event`) that other cards on the table can react to (e.g. *Barrel*).
7. **Notification** — Every change is recorded via `game_net_manager::add_update`, JSON-serialized and distributed to the relevant clients (`update_target`, a `player_set` filtering who should receive the update — useful for hiding information, e.g. which card another player drew in certain modes).
8. **Bot cycle** — If the turn or response falls to a bot, `bot_ai.cpp` automatically generates and verifies a plausible action following the same validation path (`verify_and_play`), guaranteeing that bots follow exactly the same rules as human players.

---

## 10. Build system

- **CMake ≥ 3.13**, **C++23** standard required.
- Minimal external dependencies, some vendored under `external/` (`rapidjson`, `uwebsockets`, a `reflect` library for the compile-time reflection used by the event/JSON system) and others resolved from the system (`libuv`, `cxxopts`, `libpng`, `SQLite3`).
- Each submodule (`net`, `game`, `cards` implicitly, `effects/<expansion>`, `target_types/<expansion>`, `config`) has its own `CMakeLists.txt`, aggregated by `src/CMakeLists.txt`.
- The Python scripts in `config/` require `PyYAML` and `Pillow` and run as part of the pre-compilation code-generation step.
- The final executable (`bangserver`/`bang-server`) links everything together via `banglibs` (a common interface for the external libraries).

---

## Quick module summary

| Module | Lines of code (~) | Responsibility in one sentence |
|---|---|---|
| `net/` | ~140K | Network, sessions, lobby, message protocol |
| `game/` | ~292K | Game engine: match state, turns, events, general rules, bot AI |
| `cards/` | ~100K | The static "vtable" framework linking YAML data and C++ implementations |
| `effects/` | ~2.4M (with data) | Implementation of every effect/character/rule, per expansion |
| `target_types/` | ~244K | Implementation of the target types valid for effects, per expansion |
| `config/` | ~608K (mostly images) | Declarative card data (YAML) + Python generators → C++ |
| `utils/` | ~148K | Generic support library (bitset, JSON, ranges, queues, RNG...) |

---

*Document generated by analyzing the state of the repository's `master` branch as of July 9, 2026.*
