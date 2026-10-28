1| # Practical examples: how to create a new card in bang-server
2| 
3| This guide builds on the two architecture documents already produced, showing **concrete use cases** for adding a card, in increasing order of complexity. For each case I indicate: what needs to change, which abstraction level it belongs to, and which parts are generic vs. card-specific.
4| 
5| General rule to keep in mind: **if the effect/target you need already exists** (even by combining several), all you need is YAML — zero C++, zero recompilation of logic. C++ is only needed when the behavior is genuinely new.
6| 
7| Important: every method on an effect/modifier/MTH handler that is part of the generated vtable must be declared `const`. This includes the usual hooks such as `on_play`, `on_prompt`, `get_error`, `can_pick`, `on_update`, `status_text`, and similar interface methods. The generated dispatch layer binds handlers through `const` references, so a non-const method will not match the expected contract and the card will not be wired up correctly.
8| 
9| ```cpp
10| struct effect_my_card {
11|     prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const {
12|         return {};
13|     }
| 
14|     void on_play(card_ptr origin_card, player_ptr origin, player_ptr target) const {
15|         // ...
16|     }
17| };
18| ```
19| 
20| ---
