#include "wsserver.h"

#include "logging.h"
#include "tracking.h"

#include "utils/json_aggregate.h"

namespace net {

    struct wsclient_data {
        std::shared_ptr<void *> client;
        std::string address;
    };

    void wsserver::init() {
        m_server.emplace<uWS::App>();
    }

#ifndef LIBUS_NO_SSL
    void wsserver::init_tls(const std::string &certificate_file, const std::string &private_key_file) {
        m_server.emplace<uWS::SSLApp>(uWS::SocketContextOptions{
            .key_file_name = private_key_file.c_str(),
            .cert_file_name = certificate_file.c_str()
        });
    }
#endif

    template<typename Function, typename Server>
    void visit_server(Function &&fn, Server &&server) {
        struct server_visitor : Function {
            using Function::operator();
            void operator()(std::monostate) {};
        };

        std::visit(server_visitor{std::forward<Function>(fn)}, std::forward<Server>(server));
    }

    template<bool SSL>
    uWS::WebSocket<SSL, true, wsclient_data> *websocket_cast(wsserver::client_handle client) {
        if (auto ptr = client.lock()) {
            return *static_cast<uWS::WebSocket<SSL, true, wsclient_data> **>(ptr.get());
        }
        return nullptr;
    }

    inline std::string_view crop_message_log(std::string_view message) {
        static constexpr size_t max_message_log_size = 1000;
        if (message.size() > max_message_log_size) {
            return message.substr(0, max_message_log_size);
        } else {
            return message;
        }
    }

    void wsserver::start(uint16_t port, bool reuse_addr) {
        int listen_options = reuse_addr ? LIBUS_LISTEN_DEFAULT : LIBUS_LISTEN_EXCLUSIVE_PORT;
        visit_server([&]<bool SSL>(uWS::CachingApp<SSL> &server) {
            server.template ws<wsclient_data>("/", {
                .maxPayloadLength = 1 * 1024 * 1024,
                .maxBackpressure = 16 * 1024 * 1024,

                .open = [this](auto *ws) {
                    wsclient_data *data = ws->getUserData();
                    logging::status("[{}] Connected", data->address = ws->getRemoteAddressAsText());
                    m_message_queue.emplace(data->client = std::make_shared<void *>(ws), connected{});
                },
                .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
                    wsclient_data *data = ws->getUserData();
                    logging::info("[{}] ==> {}", data->address, crop_message_log(message));
                    m_message_queue.emplace(data->client, std::string(message));
                },
                .close = [this](auto *ws, int code, std::string_view message) {
                    wsclient_data *data = ws->getUserData();
                    logging::status("[{}] Disconnected (code={} message={})", data->address, code, message);
                    m_message_queue.emplace(data->client, disconnected{});
                }
            })
            .get("/tracking", [this](auto *res, auto *req) {
                try {
                    auto since_date = tracking::parse_date(req->getQuery("since_date"));

                    res->writeHeader("Access-Control-Allow-Origin","*");
                    res->end(json::serialize(tracking::get_tracking_since(since_date)).dump());
                } catch (const std::exception &e) {
                    res->writeStatus("400 Bad Request");
                    res->writeHeader("Access-Control-Allow-Origin","*");
                    res->end(e.what());
                }
            })
            .listen(port, listen_options, [=, this](us_listen_socket_t *listen_socket) {
                if (listen_socket) {
                    logging::status("Server listening on port {}", port);
                } else {
                    throw std::runtime_error(std::format("Failed to listen on port {}", port));
                }
            }).run();
        }, m_server);
    }

    void wsserver::stop() {
        visit_server([&]<bool SSL>(uWS::CachingApp<SSL> &server) {
            server.getLoop()->defer([&]{
                server.close();
            });
        }, m_server);
    }

    void wsserver::tick() {
        while (auto elem = m_message_queue.pop()) {
            const auto &[client, message] = *elem;

            std::visit(overloaded {
                [&](std::string_view str) { on_message(client, str); },
                [&](connected) { on_connect(client); },
                [&](disconnected) { on_disconnect(client); }
            }, message);
        }
    }

    void wsserver::push_message(client_handle client, std::string message) {
        visit_server([&]<bool SSL>(uWS::CachingApp<SSL> &server) {
            if (auto *ws = websocket_cast<SSL>(client)) {
                server.getLoop()->defer([ws, message = std::move(message)]{
                    auto *data = ws->getUserData();
                    logging::info("[{}] <== {}", data->address, crop_message_log(message));
                    ws->send(message, uWS::TEXT);
                });
            }
        }, m_server);
    }

    void wsserver::kick_client(client_handle client, std::string message) {
        visit_server([&]<bool SSL>(uWS::CachingApp<SSL> &server) {
            if (auto *ws = websocket_cast<SSL>(client)) {
                server.getLoop()->defer([ws, message = std::move(message)]{
                    ws->end(1000, message);
                });
            }
        }, m_server);
    }

}