#include <charconv>
#include <iostream>
#include <signal.h>

#include <cxxopts.hpp>

#include "manager.h"

volatile bool g_stop = false;

int main(int argc, char **argv) {
    asio::io_context ctx;

    banggame::game_manager server(ctx);

    cxxopts::Options options(argv[0], "Bang! Server");

    uint16_t port = banggame::default_server_port;

    options.add_options()
        ("port",        "",                 cxxopts::value(port))
        ("cheats",      "Enable Cheats",    cxxopts::value(server.options().enable_cheats))
        ("v,verbose",   "Verbose Logging",  cxxopts::value(server.options().verbose))
        ("h,help",      "Print Help")
    ;

    options.positional_help("Port Number");
    options.parse_positional({"port"});

    try {
        auto results = options.parse(argc, argv);

        if (results.count("help")) {
            fmt::print("{}\n", options.help());
            return 0;
        }
    } catch (const std::exception &error) {
        fmt::print("Invalid arguments: {}\n", error.what());
        return 1;
    }

    if (server.start(port)) {
        fmt::print("Server listening on port {}\n", port);
        fflush(stdout);

        ::signal(SIGTERM, [](int) {
            g_stop = true;
        });

        auto next_tick = std::chrono::steady_clock::now() + banggame::ticks64{0};

        while (!g_stop) {
            next_tick += banggame::ticks64{1};

            ctx.poll();
            server.tick();

            std::this_thread::sleep_until(next_tick);
        }

        fmt::print("Server stopped\n");

        return 0;
    } else {
        fmt::print(stderr, "Could not start server\n");
        return 1;
    }
}