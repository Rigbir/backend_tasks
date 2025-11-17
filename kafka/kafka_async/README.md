# Kafka Async

C++ application demonstrating asynchronous Kafka Producer and Consumer using librdkafka with worker threads.

## Architecture

```
┌──────────────────────┐         ┌──────────────────────┐
│ Async_Kafka_Producer │         │ Async_Kafka_Consumer │
│                      │         │                      │
│  ┌──────────────┐    │         │  ┌──────────────┐    │
│  │ worker_loop  │    │         │  │ worker_loop  │    │
│  │   (thread)   │    │         │  │   (thread)   │    │
│  └──────┬───────┘    │         │  └──────┬───────┘    │
│         │            │         │         │            │
│  ┌──────▼───────┐    │         │  ┌──────▼───────┐    │
│  │   queue_     │    │         │  │   queue_     │    │
│  │  (thread-    │    │         │  │  (thread-    │    │
│  │   safe)      │    │         │  │   safe)      │    │
│  └──────┬───────┘    │         │  └──────┬───────┘    │
│         │            │         │         │            │
│  send() │            │         │ poll_   │            │
│  └──────┴────────────┘         │ message()            │
│         │                      │  └───────────────────┘
│         │ TCP/IP               │         │ TCP/IP
│         │ Kafka Protocol       │         │ Kafka Protocol
│         │                      │         │
│         └──────────┬───────────┴─────────┘
│                    │
│                    ▼
│           ┌────────────────┐
│           │  Kafka Server  │
│           │   (Docker)     │
│           │   Port 9092    │
│           │   KRaft Mode   │
│           └────────────────┘
```

## API

### Async_Kafka_Producer

- `send(const std::string& payload)` - sends message to topic (non-blocking, async)
- `flush()` - waits for all messages to be delivered (5s timeout)
- `stop()` - stops worker thread and flushes remaining messages

### Async_Kafka_Consumer

- `poll_message()` - reads one message from topic (non-blocking, 100ms timeout)
- Returns message string or empty string if timeout/no message
- `stop()` - stops worker thread

## Dependencies

- librdkafka (C/C++ Kafka client)
  - macOS: `brew install librdkafka`
  - Linux: `sudo apt-get install librdkafka-dev`
- C++17 or higher
- CMake 3.31 or higher

## Build

```bash
mkdir -p build
cd build
cmake ..
make
```

## Run

1. Start Kafka server:
```bash
docker-compose up -d
```

2. Run the demo:
```bash
./build/kafka_async
```