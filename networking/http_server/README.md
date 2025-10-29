# HTTP Server

HTTP server with routing support and concurrent request processing via thread pool.

## Architecture

The `HttpServer` class encapsulates server logic:
- Thread pool of 4 threads for parallel request processing
- HTTP request parsing into `HttpRequest` structure (method, path, headers, body)
- HTTP response formation through `HttpResponse` structure
- Request path routing with corresponding response return

## Components

- `HttpServer` - main server class
  - `start()` - starts connection acceptance loop and posts tasks to thread pool
  - `handle_client()` - processes client HTTP requests
  - `route()` - request path routing
  - `requestInfo()` - request logging with timestamps

- `HttpRequest` - incoming request parsing structure
  - `parse_request()` - parses HTTP string into components

- `HttpResponse` - response formation structure
  - `to_string()` - converts to HTTP response string

## Routes

- `GET /hello` - returns "Hello World"
- `GET /json` - returns JSON object `{"msg":"hi"}`
- All other paths - 404 Not Found

## Technologies

- C++20
- Boost.Asio for network operations
- boost::asio::thread_pool for parallel processing
- std::map for request headers storage
