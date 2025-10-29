# Echo Server

TCP echo server that returns sent data to clients.

## Architecture

The server uses a multi-threaded connection handling model:
- Main thread accepts incoming connections via `tcp::acceptor`
- Each connection is processed in a separate detached thread
- The `handler` function reads data from socket and sends it back

## Components

- `main()` - server initialization on port 8080, connection acceptance loop
- `handler(tcp::socket)` - client connection handling in read-echo-write cycle

## Technologies

- C++20
- Boost.Asio for asynchronous network operations
- std::thread for multithreading
