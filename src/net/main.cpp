#include <charconv>
#include <csignal>

#include <cxxopts.hpp>

#include "manager.h"

volatile bool g_stop = false;

void handle_stop(int signal) {
    g_stop = true;
}

int main(int argc, char **argv) {
    banggame::game_manager server;

    cxxopts::Options options(argv[0], "Bang! Server");

    uint16_t port = banggame::default_server_port;
    bool reuse_addr = false;

#ifndef WSSERVER_NO_TLS
    bool enable_tls = false;
    std::string certificate_file;
    std::string private_key_file;
#endif

    options.add_options()
        ("port",        "",                 cxxopts::value(port))
        ("cheats",      "Enable Cheats",    cxxopts::value(server.options().enable_cheats))
        ("l,logging",   "Logging Level",    cxxopts::value(logging::log_function::global_level))
        ("r,reuse-addr","Reuse Address",    cxxopts::value(reuse_addr))
#ifndef WSSERVER_NO_TLS
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

    try {
#ifndef WSSERVER_NO_TLS
        if (enable_tls) {
            server.init_tls(certificate_file, private_key_file);
        } else {
            server.init();
        }
#else
        server.init();
#endif
        server.start(port, reuse_addr);
        logging::status("Server listening on port {}", port);

        std::signal(SIGTERM, handle_stop);
        std::signal(SIGINT, handle_stop);

        auto next_tick = std::chrono::steady_clock::now() + banggame::ticks64{0};

        while (!g_stop) {
            next_tick += banggame::ticks64{1};
            server.tick();
            std::this_thread::sleep_until(next_tick);
        }

        logging::status("Stopping server...");
        
        server.stop();
        logging::status("Server stopped");

        return 0;
    } catch (const std::system_error &error) {
        std::cerr << "Could not start server: " << error.what() << '\n';
    } catch (const std::exception &error) {
        std::cerr << "Unhandled exception: " << error.what() << '\n';
    }
    return 1;
}