//
// Created by Marat on 16.11.25.
//

#pragma once

#include <librdkafka/rdkafkacpp.h>
#include <memory>
#include <string>

class Kafka_Producer {
public:
    explicit Kafka_Producer(const std::string& brokers, const std::string& topic);
    void send(const std::string& payload) const;
    void flush() const;

private:
    std::unique_ptr<RdKafka::Producer> producer_;
    std::unique_ptr<RdKafka::Topic> topic_;
};



