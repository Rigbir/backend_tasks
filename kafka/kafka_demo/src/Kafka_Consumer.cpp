//
// Created by Marat on 16.11.25.
//

#include "Kafka_Consumer.h"
#include <iostream>

Kafka_Consumer::Kafka_Consumer(const std::string &brokers,
                               const std::string &group,
                               const std::string &topic) {
    std::string err;

    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    conf->set("bootstrap.servers", brokers, err);
    conf->set("group.id", group, err);
    conf->set("auto.offset.reset", "earliest", err);

    consumer_.reset(RdKafka::KafkaConsumer::create(conf, err));
    if (!consumer_) {
        throw std::runtime_error("Consumer creation failed: " + err);
    }

    consumer_->subscribe({topic});
    delete conf;
}

std::string Kafka_Consumer::poll_message() {
    std::unique_ptr<RdKafka::Message> msg(consumer_->consume(200));

    if (msg->err() == RdKafka::ERR_NO_ERROR) {
        return { static_cast<const char*>(msg->payload()), msg->len() };
    } else if (msg->err() == RdKafka::ERR__TIMED_OUT) {
        return {};
    } else if (msg->err() == RdKafka::ERR__PARTITION_EOF) {
        return {};
    } else {
        std::cerr << "[CONSUMER ERROR] " << msg->errstr() << " (code: " << msg->err() << ")\n";
        return {};
    }
}

