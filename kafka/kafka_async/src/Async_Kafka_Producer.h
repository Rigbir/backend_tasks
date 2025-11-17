//
// Created by Marat on 17.11.25.
//

#pragma once

#include <mutex>
#include <queue>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <librdkafka/rdkafkacpp.h>

class Async_Kafka_Producer {
public:
    explicit Async_Kafka_Producer(const std::string& brokers,
                                  const std::string& topic);

    ~Async_Kafka_Producer();

    Async_Kafka_Producer(const Async_Kafka_Producer& other) = delete;
    Async_Kafka_Producer(Async_Kafka_Producer&& other) = delete;
    Async_Kafka_Producer& operator=(const Async_Kafka_Producer& other) = delete;
    Async_Kafka_Producer& operator=(Async_Kafka_Producer&& other) = delete;

    void send(const std::string& payload);
    void flush() const;
    void stop();

private:
    void worker_loop();

    struct Message {
        std::string payload;
        RdKafka::ErrorCode error_code;
    };

private:
    std::thread worker_;
    std::atomic<bool> stop_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<Message> queue_;

    std::unique_ptr<RdKafka::Producer> producer_;
    std::unique_ptr<RdKafka::Topic> topic_;
};
