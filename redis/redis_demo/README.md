# Redis Demo

HTTP server with Redis integration for key-value storage operations.

## Architecture

```
         ┌─────────────┐
         │   Client    │
         │  (Browser/  │
         │    curl)    │
         └──────┬──────┘
                │ HTTP Request
                │ GET /key or POST /
                ▼
┌─────────────────────────────────────┐
│         Http_Server                 │
│  ┌──────────────────────────────┐   │
│  │  Boost.Beast HTTP Handler    │   │
│  │  - Parse request             │   │
│  │  - Route to do_action()      │   │
│  │  - Format response           │   │
│  └──────────┬───────────────────┘   │
│             │                       │
│             │ get(key) / set(k,v)   │
│             ▼                       │
│  ┌──────────────────────────────┐   │
│  │      Redis_Client            │   │
│  │  (hiredis wrapper)           │   │
│  │  - redisConnect()            │   │
│  │  - redisCommand()            │   │
│  │  - RAII cleanup              │   │
│  └──────────┬───────────────────┘   │
└─────────────┼───────────────────────┘
              │ TCP/IP
              │ Redis Protocol
              ▼
       ┌──────────────┐
       │ Redis Server │
       │  (Docker)    │
       │  Port 6379   │
       └──────────────┘
```

**Request Flow:**
1. Client sends HTTP request (GET/POST)
2. `Http_Server` accepts connection and parses request
3. `do_action()` routes to Redis operations
4. `Redis_Client` executes Redis commands via hiredis
5. Response sent back to client

## API

- `GET /<key>` - retrieves value for the given key
  - Returns: value string or empty if key not found
  - Status: 200 OK

- `POST /` - stores key-value pair
  - Body format: `key=value`
  - Returns: "OK" on success
  - Status: 200 OK or 400 Bad Request

## Dependencies

- hiredis (via vcpkg)
- Boost (system, beast) (via vcpkg)

## Build

```bash
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake ..
cmake --build .
```

## Run

1. Start Redis server:
```bash
docker-compose up -d
```

2. Run the server:
```bash
./cmake-build-debug/redis_demo
```

3. Test with curl:
```bash
# Set value
curl -X POST http://localhost:8080/ -d "mykey=myvalue"

# Get value
curl http://localhost:8080/mykey
```

