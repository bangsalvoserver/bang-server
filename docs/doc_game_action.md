# Oggetto game_action

### Quando 'selezioni' una carta
    {"pick_card": {
        "card": 123, // id carta
        "bypass_prompt": false
    }}

### Quando 'giochi' una carta
    {"play_card": {
        "card": 123, // id carta
        "targets": [ { /* oggetto play_card_target */ } ],
        "modifiers": [
            {
                "card": 124,
                "targets": [ { /* oggetto play_card_target */ } ]
            }
        ],
        "bypass_prompt": false
    }}

Ogni carta contiene tre liste di "effetti": `effects`, `responses` e `optionals`. Un effetto contiene un tipo e un target, che puo' essere uno dei seguenti:

### Oggetto play_card_target
    {"none": {}} // nessun target
    {"player": 3} // id di player
    {"conditional_player": 3} // id di player, puo' essere null
    {"card": 123} // id di carta
    {"extra_card": 123} // id di carta, puo' essere null
    {"players": {}} // target a tutti i giocatori
    {"cards": [ 123, 124, 125 ]} // id di carte
    {"cards_other_players": [ 123, 124, 125 ]} // id di carte, una per giocatore
    {"select_cubes": [ 123, 123, 124 ]} // id di carte per selezionare cubetti
    {"self_cubes": {}}

Il tipo del play_card_target deve corrispondere al tipo dell'effetto della carta che stai giocando

Il campo `modifier` serve per selezionare i target per effetti di carte combinate. Per esempio se giochi mira + bang `card` e' la carta bang, `targets` e' la lista di target per la carta bang e `modifiers` e' la lista con la carta mira + la lista di target per mira

Il campo `bypass_prompt` serve per ignorare il check di prompt lato server. Se settato su false il server puo' rispondere con un messaggio di tipo game_prompt. Per rispondere Si' bisogna rimandare lo stesso game_action con `bypass_prompt=true`