#include <charconv>
#include <csignal>
#include <thread>

#include <cxxopts.hpp>

#include "manager.h"
#include "tracking.h"

std::stop_source g_stop;

void handle_stop(int signal = 0) {
    if (g_stop.stop_possible()) {
        g_stop.request_stop();
    }
}

int main(int argc, char **argv) {
    banggame::game_manager server;

    cxxopts::Options options(argv[0], "Bang! Server");

    uint16_t port = banggame::default_server_port;
    bool reuse_addr = false;

    std::string tracking_file;

#ifndef LIBUS_NO_SSL
    bool enable_tls = false;
    std::string certificate_file;
    std::string private_key_file;
#endif

    options.add_options()
        ("port",        "",                 cxxopts::value(port))
        ("cheats",      "Enable Cheats",    cxxopts::value(server.options().enable_cheats))
        ("l,logging",   "Logging Level",    cxxopts::value(logging::log_function::global_level))
        ("r,reuse-addr","Reuse Address",    cxxopts::value(reuse_addr))
        ("t,tracking-db","Tracking Database File", cxxopts::value(tracking_file))
#ifndef LIBUS_NO_SSL
        ("s,secure",    "Enable TLS",       cxxopts::value(enable_tls))
        ("cert",        "Certificate File", cxxopts::value(certificate_file))
        ("key",         "Private Key File", cxxopts::value(private_key_file))
#endif
        ("h,help",      "Print Help")
    ;

    options.positional_help("Port Number");
    options.parse_positional({"port"});

    try {
        auto results = options.parse(argc, argv);

        if (results.count("help")) {
            std::cout << options.help();
            return 0;
        }
    } catch (const std::exception &error) {
        std::cerr << "Invalid arguments: " << error.what() << '\n';
        return 1;
    }

    if (!tracking_file.empty()) {
        tracking::init_tracking(tracking_file);
    }

#ifndef LIBUS_NO_SSL
    if (enable_tls) {
        server.init_tls(certificate_file, private_key_file);
    } else {
        server.init();
    }
#else
    server.init();
#endif

    std::atomic_bool server_error = false;

    std::jthread main_loop{[&](std::stop_token stop) {
        tracking::track_zero();

        try {
            auto next_tick = std::chrono::steady_clock::now() + banggame::ticks64{0};

            while (!stop.stop_requested()) {
                next_tick += banggame::ticks64{1};
                server.tick();
                std::this_thread::sleep_until(next_tick);
            }
        } catch (const std::exception &error) {
            std::cerr << "Unhandled exception: " << error.what() << '\n';
        }

        if (!server_error) {
            logging::status("Stopping server...");
            server.stop();
            logging::status("Server stopped");
        }

        tracking::track_zero();
    }};

    g_stop = main_loop.get_stop_source();

    std::signal(SIGTERM, handle_stop);
    std::signal(SIGINT, handle_stop);
    
    try {
        server.start(port, reuse_addr);
    } catch (const std::exception &error) {
        std::cerr << "Could not start server: " << error.what() << '\n';
        server_error = true;
        handle_stop();
    }

    return 0;
}