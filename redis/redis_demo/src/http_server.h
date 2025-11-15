//
// Created by Marat on 15.11.25.
//

#pragma once

#include <memory>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "redis_client.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class Http_Server {
public:
    explicit Http_Server(int port, std::shared_ptr<Redis_Client> redis_client);
    void run();

private:
    void handle_session(tcp::socket socket) const;
    void do_action(const http::request<http::string_body>& req, http::response<http::string_body>& res) const;


private:
    int port_;
    net::io_context io_;
    std::shared_ptr<Redis_Client> redis_client_;
};



