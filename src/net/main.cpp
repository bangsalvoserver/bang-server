#include <charconv>
#include <iostream>
#include <signal.h>

#include "wsbang.h"

volatile bool g_stop = false;

int main(int argc, char **argv) {
    asio::io_context ctx;

    banggame::bang_server server(ctx);

    uint16_t port = banggame::default_server_port;
    if (argc > 1) {
        auto [ptr, ec] = std::from_chars(argv[1], argv[1] + strlen(argv[1]), port);
        if (ec != std::errc{}) {
            std::cerr << "Port must be a number" << std::endl;
            return 1;
        }
    }

    if (server.start(port)) {
        std::cout << "Server listening on port " << port << std::endl;

        ::signal(SIGTERM, [](int) {
            g_stop = true;
        });

        using ticks = std::chrono::duration<int64_t, std::ratio<1, banggame::server_tickrate>>;
        auto next_tick = std::chrono::steady_clock::now() + ticks{0};

        while (!g_stop) {
            next_tick += ticks{1};

            ctx.poll();
            server.tick();

            std::this_thread::sleep_until(next_tick);
        }

        std::cout << "Server stopped" << std::endl;

        return 0;
    } else {
        std::cerr << "Could not start server" << std::endl;
        return 1;
    }
}