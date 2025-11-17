//
// Created by Marat on 17.11.25.
//

#pragma once

#include <queue>
#include <mutex>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <librdkafka/rdkafkacpp.h>

class Async_Kafka_Consumer {
public:
    explicit Async_Kafka_Consumer(const std::string& brokers,
                                  const std::string& group_id,
                                  const std::string& topics);

    ~Async_Kafka_Consumer();

    Async_Kafka_Consumer(const Async_Kafka_Consumer& other) = delete;
    Async_Kafka_Consumer(Async_Kafka_Consumer&& other) = delete;
    Async_Kafka_Consumer& operator=(const Async_Kafka_Consumer& other) = delete;
    Async_Kafka_Consumer& operator=(Async_Kafka_Consumer&& other) = delete;

    std::string poll_message();
    void stop();

private:
    void worker_loop();

private:
    std::thread worker_;
    std::atomic<bool> stop_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::string> queue_;

    std::unique_ptr<RdKafka::KafkaConsumer> consumer_;
};