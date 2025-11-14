#include "api/HttpServer.h"
#include "util/Logger.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

void HttpServer::Start(int port) {
    using tcp  = boost::asio::ip::tcp;
    namespace http = boost::beast::http;
    namespace beast = boost::beast;

    try {
        boost::asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {tcp::v4(), static_cast<unsigned short>(port)}};

        util::Logger::Info("HTTP server started on port " + std::to_string(port));

        for (;;) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::read(socket, buffer, req);

            std::string target = std::string(req.target());
            std::string body   = req.body();

            std::string responseBody = R"({"status":"ERROR","error_code":"NOT_FOUND"})";
            auto it = handlers_.find(target);
            if (it != handlers_.end()) {
                responseBody = it->second(body);
            }

            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::server, "SlotBackend");
            res.set(http::field::content_type, "application/json");
            res.keep_alive(false);
            res.body() = responseBody;
            res.prepare_payload();

            http::write(socket, res);

            beast::error_code ec;
            socket.shutdown(tcp::socket::shutdown_send, ec);
        }
    } catch (const std::exception& ex) {
        util::Logger::Error(std::string("HTTP server error: ") + ex.what());
        throw;
    }
}
