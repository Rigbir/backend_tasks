#include <boost/beast/core/detail/base64.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <openssl/sha.h>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <array>
#include <mutex>

using boost::asio::ip::tcp;

class WebSocketServer {
public:
    WebSocketServer(boost::asio::io_context& io, unsigned short port)
        : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "Server start on port: " << port << '\n';
    }

    void start() {
        try {
            for (;;) {
                tcp::socket socket(io_);
                acceptor_.accept(socket);

                std::thread([this, s = std::move(socket)]() mutable {
                    handle_handshake(std::move(s));
                }).detach();
            }
        } catch (std::exception& e) {
            std::cout << "Server error: " << e.what() << '\n';
        }
    }

private:
    boost::asio::io_context& io_;
    tcp::acceptor acceptor_;

    inline static std::mutex clients_mutex;
    inline static std::atomic<int> client_count;
    inline static std::vector<std::shared_ptr<tcp::socket>> clients;

    static void handle_handshake(tcp::socket socket) {
        try {
            std::array<char, 1024> data{};
            std::size_t length = socket.read_some(boost::asio::buffer(data));

            std::string request(data.data(), length);
            std::cout << "Handshake request:\n" << request << '\n';

            if (request.find("Upgrade: websocket") == std::string::npos) {
                std::cout << "Not a WebSocket request\n";
                return;
            }

            std::string key;
            std::istringstream stream(request);
            std::string line;
            while (std::getline(stream, line)) {
                if (line.starts_with("Sec-WebSocket-Key:")) {
                    key = line.substr(18);
                    key.erase(std::ranges::remove_if(key, ::isspace).begin(), key.end());
                    break;
                }
            }

            if (key.empty()) {
                std::cout << "Missing Sec-WebSocket-Key\n";
                return;
            }

            std::string accept_key = generate_accept_key(key);

            std::ostringstream response;
            response << "HTTP/1.1 101 Switching Protocols\r\n"
                     << "Upgrade: websocket\r\n"
                     << "Connection: Upgrade\r\n"
                     << "Sec-WebSocket-Accept: " << accept_key << "\r\n\r\n";

            boost::asio::write(socket, boost::asio::buffer(response.str()));

            auto client_ptr = std::make_shared<tcp::socket>(std::move(socket));
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.push_back(client_ptr);
            }
            ++client_count;
            std::cout << "Handshake complete\n";
            std::cout << "Client count: " << client_count << '\n';

            handle_frames(client_ptr);

            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.erase(std::ranges::remove(clients, client_ptr).begin(), clients.end());
            }
            --client_count;
            std::cout << "Client disconnected. Client count: " << client_count << '\n';

        } catch (std::exception& e) {
            std::cout << "Handshake error: " << e.what() << '\n';
        }
    }

    static void handle_frames(const std::shared_ptr<tcp::socket>& client_ptr) {
        try {
            std::array<unsigned char, 1024> data{};

            while (client_ptr->is_open()) {
                std::size_t length = client_ptr->read_some(boost::asio::buffer(data));
                WSFrame frame = decode_frame(data.data(), length);

                std::cout << "Client sent: " << frame.payload << '\n';

                std::string response = "Server got: " + frame.payload;

                std::lock_guard<std::mutex> lock(clients_mutex);
                for (auto it = clients.begin(); it != clients.end(); ) {
                    try {
                        boost::asio::write(**it, boost::asio::buffer(encode_frame(response)));
                        ++it;
                    } catch (...) {
                        it = clients.erase(it);
                    }
                }
            }
        } catch (std::exception& e) {
            std::cout << "Handle frames error: " << e.what() << '\n';
        }
    }

    static std::string generate_accept_key(const std::string& client_key) {
        static const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        std::string combined = client_key + magic;

        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);

        std::size_t encoded_size = boost::beast::detail::base64::encoded_size(SHA_DIGEST_LENGTH);

        std::string encoded(encoded_size, '\0');
        boost::beast::detail::base64::encode(&encoded[0], hash, SHA_DIGEST_LENGTH);

        return encoded;
    }

    struct WSFrame {
        bool fin;
        unsigned char opcode;
        std::string payload;
    };

    static WSFrame decode_frame(const unsigned char* data, std::size_t length) {
        WSFrame frame;
        if (length < 2) throw std::runtime_error("Frame too short");

        unsigned char fin_rsv_opcode = data[0];
        unsigned char mask_payload_len = data[1];

        frame.fin = fin_rsv_opcode & 0x80;
        frame.opcode = fin_rsv_opcode & 0x0F;

        std::size_t payload_len = mask_payload_len & 0x7F;
        std::size_t header_size = 2;

        std::array<unsigned char, 4> mask_key{};
        if (payload_len == 126) {
            payload_len = (data[2] << 8) | data[3];
            header_size += 2;
        } else if (payload_len == 127) {
            throw std::runtime_error("Payload too big");
        }

        bool masked = mask_payload_len & 0x80;
        if (masked) {
            for (int i = 0; i < 4; ++i) mask_key[i] = data[header_size + i];
            header_size += 4;
        }

        frame.payload.resize(payload_len);
        for (std::size_t i = 0; i < payload_len; ++i) {
            unsigned char byte = data[header_size + i];
            if (masked) byte ^= mask_key[i % 4];
            frame.payload[i] = byte;
        }

        return frame;
    }

    static std::vector<unsigned char> encode_frame(const std::string& msg) {
        std::vector<unsigned char> out;
        out.push_back(0x81);
        if (msg.size() < 126) {
            out.push_back(msg.size());
        } else {
            out.push_back(126);
            out.push_back((msg.size() >> 8) & 0xFF);
            out.push_back(msg.size() & 0xFF);
        }
        out.insert(out.end(), msg.begin(), msg.end());
        return out;
    }
};

int main() {
    boost::asio::io_context io;
    WebSocketServer server(io, 8080);
    server.start();
}