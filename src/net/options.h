#ifndef __NET_OPTIONS_H__
#define __NET_OPTIONS_H__

#include <chrono>

namespace banggame {
    using namespace std::chrono_literals;

    constexpr uint16_t default_server_port = 47654;
    constexpr int server_max_clients = 100;
    constexpr int lobby_max_players = 8;
    constexpr int server_tickrate = 120;

    template<std::integral T>
    using ticks_t = std::chrono::duration<T, std::ratio<1, server_tickrate>>;
    
    using ticks = ticks_t<int>;
    using ticks64 = ticks_t<int64_t>;

}

#endif