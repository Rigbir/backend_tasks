//
// Created by Marat on 16.11.25.
//

#pragma once

#include <librdkafka/rdkafkacpp.h>
#include <memory>
#include <string>

class Kafka_Consumer {
public:
    explicit Kafka_Consumer(const std::string& brokers,
                            const std::string& group,
                            const std::string& topic);

    std::string poll_message();

private:
    std::unique_ptr<RdKafka::KafkaConsumer> consumer_;
};
