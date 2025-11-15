//
// Created by Marat on 15.11.25.
//

#include "http_server.h"
#include <iostream>

Http_Server::Http_Server(int port, std::shared_ptr<Redis_Client> redis_client)
    : port_(port)
    , redis_client_(redis_client) {}

void Http_Server::run() {
    try {
        tcp::acceptor acceptor(io_, tcp::endpoint(tcp::v4(), port_));

        std::cout << "HTTP Server started on port " << port_ << '\n';

        for (;;) {
            tcp::socket socket(io_);
            acceptor.accept(socket);
            handle_session(std::move(socket));
        }
    } catch (const std::exception& e) {
        std::cout << "Server error:" << e.what() << '\n';
    }
}

void Http_Server::handle_session(boost::asio::ip::tcp::socket socket) const {
    beast::flat_buffer buffer;

    http::request<http::string_body> req;
    http::read(socket, buffer, req);

    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::content_type, "text/plain");

    do_action(req, res);

    res.prepare_payload();
    http::write(socket, res);
    socket.shutdown(tcp::socket::shutdown_send);
}

void Http_Server::do_action(const http::request<http::string_body>& req, http::response<http::string_body>& res) const {
    if (req.method() == http::verb::get) {
        std::string key(req.target());
        if (!key.empty() && key[0] == '/') {
            key.erase(0, 1);
        }

        auto value = redis_client_->get(key);
        res.result(http::status::ok);
        res.body() = value;
    } else if (req.method() == http::verb::post) {
        std::string body = req.body();
        auto pos = body.find('=');

        if (pos != std::string::npos) {
            std::string key = body.substr(0, pos);
            std::string value = body.substr(pos + 1);

            redis_client_->set(key, value);

            res.result(http::status::ok);
            res.body() = "OK";
        } else {
            res.result(http::status::bad_request);
            res.body() = "Bad body format";
        }
    } else {
        res.result(http::status::bad_request);
        res.body() = "Unsupported method";
    }
}