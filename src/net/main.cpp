#include <charconv>
#include <iostream>
#include <signal.h>

#include <cxxopts.hpp>

#include "wsbang.h"

volatile bool g_stop = false;

int main(int argc, char **argv) {
    asio::io_context ctx;

    banggame::bang_server server(ctx);

    cxxopts::Options options(argv[0], "Bang! Server");

    uint16_t port = banggame::default_server_port;

    options.add_options()
        ("port",        "",                 cxxopts::value(port))
        ("cheats",      "Enable Cheats",    cxxopts::value(server.m_mgr.options().enable_cheats))
        ("v,verbose",   "Verbose Logging",  cxxopts::value(server.m_mgr.options().verbose))
        ("h,help",      "Print Help")
    ;

    options.positional_help("Port Number");
    options.parse_positional({"port"});

    try {
        auto results = options.parse(argc, argv);

        if (results.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }
    } catch (const std::exception &error) {
        std::cout << "Invalid arguments: " << error.what() << "\n";
        return 1;
    }

    if (server.start(port)) {
        std::cout << "Server listening on port " << port << std::endl;

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

        std::cout << "Server stopped" << std::endl;

        return 0;
    } else {
        std::cerr << "Could not start server" << std::endl;
        return 1;
    }
}