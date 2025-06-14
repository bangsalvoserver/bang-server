#include "wsserver.h"

#include "logging.h"
#include "tracking.h"
#include "image_registry.h"

#include "utils/json_aggregate.h"
#include "utils/parse_string.h"
#include "utils/misc.h"

#include <stdexcept>

#include <App.h>

namespace net {

    class wsserver_impl {
    public:
        std::variant<
            uWS::App
#ifndef LIBUS_NO_SSL
            , uWS::SSLApp
#endif
        > app;

        template<typename T, typename ... Ts>
        wsserver_impl(std::in_place_type_t<T> tag, Ts && ... args)
            : app{tag, std::forward<Ts>(args) ... } {}
    };

    void wsserver_impl_deleter::operator()(wsserver_impl *ptr) const {
        delete ptr;
    }

    template<typename Function>
    void visit_server(Function &&fn, std::unique_ptr<wsserver_impl, wsserver_impl_deleter> &server) {
        if (server) {
            std::visit(std::forward<Function>(fn), server->app);
        }
    }

    void wsserver::init() {
        m_server.reset(new wsserver_impl(std::in_place_type<uWS::App>));
    }

#ifndef LIBUS_NO_SSL
    void wsserver::init_tls(const std::string &certificate_file, const std::string &private_key_file) {
        m_server.reset(new wsserver_impl(std::in_place_type<uWS::SSLApp>, uWS::SocketContextOptions{
            .key_file_name = private_key_file.c_str(),
            .cert_file_name = certificate_file.c_str()
        }));
    }
#endif

    struct wsclient_data {
        std::shared_ptr<void *> client;
        std::string address;
    };

    template<bool SSL>
    static uWS::WebSocket<SSL, true, wsclient_data> *websocket_cast(wsserver::client_handle client) {
        if (auto ptr = client.lock()) {
            return *static_cast<uWS::WebSocket<SSL, true, wsclient_data> **>(ptr.get());
        }
        return nullptr;
    }

    void wsserver::start(uint16_t port, bool reuse_addr) {
        int listen_options = reuse_addr ? LIBUS_LISTEN_DEFAULT : LIBUS_LISTEN_EXCLUSIVE_PORT;
        visit_server([&]<bool SSL>(uWS::TemplatedApp<SSL> &server) {
            server.template ws<wsclient_data>("/", {
#ifndef UWS_NO_ZLIB
                .compression = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR_4KB | uWS::DEDICATED_DECOMPRESSOR),
#endif
                .maxPayloadLength = 1 * 1024 * 1024,

                .open = [this](auto *ws) {
                    wsclient_data *data = ws->getUserData();
                    logging::status("[{}] Connected", data->address = ws->getRemoteAddressAsText());
                    m_message_queue.emplace(data->client = std::make_shared<void *>(ws), connected{});
                },
                .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
                    wsclient_data *data = ws->getUserData();
                    logging::info("[{}] ==> {:.{}}", data->address, message, max_message_log_size);
                    m_message_queue.emplace(data->client, std::string(message));
                },
                .close = [this](auto *ws, int code, std::string_view message) {
                    wsclient_data *data = ws->getUserData();
                    logging::status("[{}] Disconnected (code={} message={})", data->address, code, message);
                    m_message_queue.emplace(data->client, disconnected{});
                }
            })
            .get("/.env", [this](auto *res, auto *req) {
                res->writeStatus("418 I'm a teapot");
                res->end("nice try :p");
                logging::warn("[{}] is trying to hack the server", res->getRemoteAddressAsText());
            })
            .get("/tracking", [this](auto *res, auto *req) {
                auto length = tracking::parse_length(req->getQuery("length"));
                auto max_count = utils::parse_string<size_t>(req->getQuery("max_count"));
                if (length) {
                    res->writeHeader("Access-Control-Allow-Origin","*");
                    res->writeHeader("Content-Type", "application/json");
                    res->end(json::serialize(tracking::get_tracking_for(*length, max_count.value_or(3000))).dump());
                } else {
                    res->writeStatus("400 Bad Request");
                    res->writeHeader("Access-Control-Allow-Origin","*");
                    res->end(length.error());
                }
            })
            .get("/image/:hash", [this](auto *res, auto *req) {
                if (auto hash = utils::parse_string<size_t>(req->getParameter("hash"), 16)) {
                    auto [guard, bytes] = banggame::image_registry::get_png_image_data(*hash);
                    if (!bytes.empty()) {
                        res->writeStatus("200 OK");
                        res->writeHeader("Content-Type", "image/png");
                        res->end({ reinterpret_cast<const char*>(bytes.data()), bytes.size() });
                    } else {
                        res->writeStatus("404 Not Found");
                        res->end();
                    }
                } else {
                    res->writeStatus("400 Bad Request");
                    res->end();
                }
            })
            .listen(port, listen_options, [=, this](us_listen_socket_t *listen_socket) {
                if (listen_socket) {
                    logging::status("Server listening on port {}", port);
                } else {
                    m_server.reset();
                    throw std::runtime_error(std::format("Failed to listen on port {}", port));
                }
            }).run();
        }, m_server);
        logging::status("Server stopped");
    }

    void wsserver::stop() {
        visit_server([&]<bool SSL>(uWS::TemplatedApp<SSL> &server) {
            logging::status("Stopping server...");
            server.getLoop()->defer([&]{
                server.close();
            });
        }, m_server);
    }

    void wsserver::poll() {
        while (auto elem = m_message_queue.pop()) {
            const auto &[client, message] = *elem;

            std::visit(overloaded {
                [&](std::string_view str) { on_message(client, str); },
                [&](connected) { on_connect(client); },
                [&](disconnected) { on_disconnect(client); }
            }, message);
        }
    }

    template<bool SSL>
    static void do_send_message(wsserver::client_handle client, std::string_view message) {
        if (auto *ws = websocket_cast<SSL>(client)) {
            auto *data = ws->getUserData();
            logging::info("[{}] <== {:.{}}", data->address, message, max_message_log_size);
            ws->send(message, uWS::TEXT);
        }
    }

    void wsserver::push_message(client_handle client, std::string message) {
        visit_server([&]<bool SSL>(uWS::TemplatedApp<SSL> &server) {
            server.getLoop()->defer([client, message = std::move(message)]{
                do_send_message<SSL>(client, message);
            });
        }, m_server);
    }

    void wsserver::push_messages(message_list messages) {
        visit_server([&]<bool SSL>(uWS::TemplatedApp<SSL> &server) {
            server.getLoop()->defer([messages = std::move(messages)]{
                for (const auto &[client, message] : messages) {
                    do_send_message<SSL>(client, message);
                }
            });
        }, m_server);
    }

    void wsserver::kick_client(client_handle client, std::string message, int code) {
        visit_server([&]<bool SSL>(uWS::TemplatedApp<SSL> &server) {
            server.getLoop()->defer([client, code, message = std::move(message)]{
                if (auto *ws = websocket_cast<SSL>(client)) {
                    ws->end(code, message);
                }
            });
        }, m_server);
    }

}