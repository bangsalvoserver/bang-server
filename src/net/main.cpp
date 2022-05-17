#include <charconv>
#include <iostream>

#include "wsbang.h"

int main(int argc, char **argv) {
    asio::io_context ctx;

    banggame::bang_server server(ctx);

    uint16_t port = banggame::default_server_port;
    if (argc > 1) {
        auto [ptr, ec] = std::from_chars(argv[1], argv[1] + strlen(argv[1]), port);
        if (ec != std::errc{}) {
            std::cerr << "Port must be a number\n";
            return 1;
        }
    }

    if (server.start(port)) {
        std::cout << "Server listening on port " << port << '\n';

        using frames = std::chrono::duration<int64_t, std::ratio<1, banggame::fps>>;
        auto next_frame = std::chrono::steady_clock::now() + frames{0};

        while (true) {
            next_frame += frames{1};

            ctx.poll();
            server.tick();

            std::this_thread::sleep_until(next_frame);
        }

        return 0;
    } else {
        std::cerr << "Could not start server\n";
        return 1;
    }
}