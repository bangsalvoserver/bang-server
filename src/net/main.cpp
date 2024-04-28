#include <charconv>
#include <iostream>
#include <csignal>

#include <cxxopts.hpp>

#include "manager.h"

#include "git_version.h"

volatile bool g_stop = false;

void handle_stop(int signal) {
    g_stop = true;
}

int main(int argc, char **argv) {
    banggame::game_manager server;

    cxxopts::Options options(argv[0], "Bang! Server");

    uint16_t port = banggame::default_server_port;

    options.add_options()
        ("port",        "",                 cxxopts::value(port))
        ("cheats",      "Enable Cheats",    cxxopts::value(server.options().enable_cheats))
        ("v,verbose",   "Verbose Logging",  cxxopts::value(server.options().verbose))
        ("h,help",      "Print Help")
#ifdef HAVE_GIT_VERSION
        ("version",   "Print Version")
#endif
    ;

    options.positional_help("Port Number");
    options.parse_positional({"port"});

    try {
        auto results = options.parse(argc, argv);

        if (results.count("help")) {
            fmt::print("{}\n", options.help());
            return 0;
        }
#ifdef HAVE_GIT_VERSION
        else if (results.count("version")) {
            fmt::print("{}\n", net::server_commit_hash);
            return 0;
        }
#endif
    } catch (const std::exception &error) {
        fmt::print("Invalid arguments: {}\n", error.what());
        return 1;
    }

    try {
        server.start(port);
        fmt::print("Server listening on port {}\n", port);
        fflush(stdout);

        std::signal(SIGTERM, handle_stop);
        std::signal(SIGINT, handle_stop);

        auto next_tick = std::chrono::steady_clock::now() + banggame::ticks64{0};

        while (!g_stop) {
            next_tick += banggame::ticks64{1};
            server.tick();
            std::this_thread::sleep_until(next_tick);
        }

        server.stop();
        fmt::print("Server stopped\n");

        return 0;
    } catch (const std::exception &error) {
        fmt::print(stderr, "Unhandled exception: {}\n", error.what());
        return 1;
    }
}