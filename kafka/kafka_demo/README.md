# Kafka Demo

C++ application demonstrating Kafka Producer and Consumer using librdkafka.

## Architecture

```
┌─────────────────┐         ┌────────────────────┐
│  Kafka_Producer │         │  Kafka_Consumer    │
│                 │         │                    │
│  - produce()    │         │  - poll_message()  │
│  - flush()      │         │  - subscribe()     │
└────────┬────────┘         └────────┬───────────┘
         │                           │
         │ TCP/IP                    │ TCP/IP
         │ Kafka Protocol            │ Kafka Protocol
         │                           │
         └───────────┬───────────────┘
                     │
                     ▼
            ┌────────────────┐
            │  Kafka Server  │
            │   (Docker)     │
            │   Port 9092    │
            │   KRaft Mode   │
            └────────────────┘
```

## API

### Kafka_Producer

- `send()` - sends message to topic (buffered, async)
- `flush()` - waits for all messages to be delivered (5s timeout)

### Kafka_Consumer

- `poll_message()` - reads one message from topic (200ms timeout)
- Returns message string or empty string if timeout/no message

## Dependencies

- librdkafka (C/C++ Kafka client)
  - macOS: `brew install librdkafka`
  - Linux: `sudo apt-get install librdkafka-dev`

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
./build/kafka_demo
```