#include <boost/asio.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <atomic>
#include <string>
#include <chrono>
#include <array>
#include <map>

using boost::asio::ip::tcp;
static std::atomic<int> client_count = 0;

class HttpServer {
public:
    HttpServer(boost::asio::io_context& io, unsigned short port)
        : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "Server started with port: " << port << '\n';
    }

    void start() {
        try {
            boost::asio::thread_pool pool(4);

            for (;;) {
                tcp::socket socket(io_);
                acceptor_.accept(socket);

                boost::asio::post(pool, [s = std::move(socket)]() mutable {
                    handle_client(std::move(s));
                });
            }
        } catch (std::exception& e) {
            std::cout << "Server error: " << e.what() << '\n';
        }
    }

private:
    boost::asio::io_context& io_;
    tcp::acceptor acceptor_;

    static void handle_client(tcp::socket socket) {
        ++client_count;
        try {
            std::array<char, 1024> data{};

            for (;;) {
                size_t length = socket.read_some(boost::asio::buffer(data));
                if (length == 0) break;

                std::string request_str(data.data(), length);
                HttpRequest request = HttpRequest::parse_request(request_str);

                HttpResponse response = route(request);
                requestInfo(request, response);

                boost::asio::write(socket, boost::asio::buffer(response.to_string()));

                if (request.headers["Connection"] == "close") break;
            }

        } catch (std::exception& e) {
            std::cout << "Client error: " << e.what() << '\n';
        }
        --client_count;
    }

    struct HttpRequest {
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;

        static HttpRequest parse_request(const std::string& request) {
            HttpRequest req;
            std::istringstream stream(request);
            std::string line;

            if (std::getline(stream, line)) {
                std::istringstream line_stream(line);
                line_stream >> req.method >> req.path >> req.version;
            }

            while (std::getline(stream, line) && line != "\r") {
                auto colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    std::string key = line.substr(0, colon_pos);
                    std::string value = line.substr(colon_pos + 1);

                    if (!value.empty() && value.back() == '\r') value.pop_back();
                    while (!value.empty() && value.front() == ' ') value.erase(value.begin());

                    req.headers[key] = value;
                }
            }

            auto it = req.headers.find("Content-Length");
            if (it != req.headers.end()) {
                size_t len = std::stoul(it->second);
                req.body.resize(len);
                stream.read(req.body.data(), len);
            }

            return req;
        }
    };

    struct HttpResponse {
        int status;
        std::string content;
        std::string content_type = "text/plain";

        std::string to_string() const {
            return
                "HTTP/1.1 " + std::to_string(status) + " " + status_text(status) + "\r\n" +
                "Content-Length: " + std::to_string(content.size()) + "\r\n" +
                "Content-Type: " + content_type + "\r\n" +
                "Connection: close\r\n\r\n" +
                content;
        }

        static std::string status_text(int code) {
            switch (code) {
                case 200: return "OK";
                case 404: return "Not Found";
                default: return "Unknown";
            }
        }
    };

    static HttpResponse route(const HttpRequest& request) {
        HttpResponse response;

        try {
            if (request.path == "/hello") {
                response = {200, "Hello World"};
            } else if (request.path == "/json") {
                response = {200, R"({"msg":"hi"})", "application/json"};
            } else {
                response = {404, "Not Found"};
            }
        } catch (...) {
            response = {500, "Internal Server Error"};
        }

        return response;
    }

    static void requestInfo(const HttpRequest& request, const HttpResponse& response) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);

        std::cout << "[" << std::put_time(std::localtime(&t), "%F %T") << "] "
                  << request.method << " " << request.path
                  << " -> " << response.status << " " << HttpResponse::status_text(response.status)
                  << " Active clients: " << client_count
                  << std::endl;
    }
};

int main() {
    boost::asio::io_context io;
    HttpServer server(io, 8080);
    server.start();
}