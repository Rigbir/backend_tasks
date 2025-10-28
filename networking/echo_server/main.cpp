#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <array>

using boost::asio::ip::tcp;
static std::atomic<int> client_count = 0;

void handler(tcp::socket socket) {
    try {
        std::array<char, 1024> data;

        for (;;) {
            std::size_t length = socket.read_some(boost::asio::buffer(data));
            boost::asio::write(socket, boost::asio::buffer(data, length));
            std::cout << "Echoed: " << std::string(data.data(), length) << '\n';
        }
    } catch (std::exception& e) {
        --client_count;
        std::cout << "Client disconnected. Total: " << client_count << '\n';
    }
}

int main() {
    try {
        boost::asio::io_context io;

        tcp::endpoint endpoint(tcp::v4(), 8080);
        tcp::acceptor acceptor(io, endpoint);

        std::cout << "Server has been connected on port 8080\n";

        for (;;) {
            tcp::socket socket(io);
            acceptor.accept(socket);

            ++client_count;
            auto client = socket.remote_endpoint();
            std::cout << "New client: " << client.address().to_string()
                      << ":" << client.port() << " | Total: " << client_count << std::endl;

            std::thread(handler, std::move(socket)).detach();
        }
    } catch (std::exception& e) {
        std::cout << "Server error: " << e.what() << '\n';
    }
}
