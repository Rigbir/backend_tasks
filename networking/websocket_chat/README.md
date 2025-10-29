# WebSocket Chat Server

WebSocket server with chat functionality: broadcasts messages to all connected clients.

## Architecture

The `WebSocketServer` class implements WebSocket protocol:
- HTTP handshake with validation and accept key generation
- WebSocket frame encoding and decoding according to RFC 6455
- Multi-threaded connection processing (detached threads)
- Centralized client list with mutex synchronization for broadcast

## Components

- `WebSocketServer` - main server class
  - `start()` - connection acceptance loop
  - `handle_handshake()` - performs HTTP upgrade handshake
  - `handle_frames()` - processes WebSocket frames from client

- `WSFrame` - decoded frame structure (fin, opcode, payload)

- `generate_accept_key()` - generates Sec-WebSocket-Accept via SHA1 + base64
- `decode_frame()` - decodes incoming WebSocket frames (masking, payload length)
- `encode_frame()` - encodes outgoing WebSocket frames (text messages, opcode 0x81)

## Protocol

1. HTTP handshake: client sends Upgrade request, server validates Sec-WebSocket-Key and returns Sec-WebSocket-Accept
2. WebSocket frames: binary protocol with client frame masking, payload support up to 126 bytes

## Technologies

- C++20
- Boost.Asio for network operations
- Boost.Beast base64 for accept key encoding
- OpenSSL SHA1 for hashing
- std::mutex for client list access synchronization
