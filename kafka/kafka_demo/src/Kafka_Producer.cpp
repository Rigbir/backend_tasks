//
// Created by Marat on 16.11.25.
//

#include "Kafka_Producer.h"
#include <iostream>

Kafka_Producer::Kafka_Producer(const std::string& brokers, const std::string& topic) {
    std::string err;

    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    conf->set("bootstrap.servers", brokers, err);
    conf->set("batch.size", "1", err);
    conf->set("linger.ms", "0", err);

    producer_.reset(RdKafka::Producer::create(conf, err));
    if (!producer_) {
        throw std::runtime_error("Producer creation failed: " + err);
    }

    RdKafka::Conf* tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    topic_.reset(RdKafka::Topic::create(producer_.get(), topic, tconf, err));

    delete conf;
    delete tconf;
}

void Kafka_Producer::send(const std::string& payload) const {
    RdKafka::ErrorCode err = producer_->produce(
        topic_.get(),
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(payload.c_str()),
        payload.size(),
        nullptr,
        nullptr
    );

    if (err != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Produce failed: " << RdKafka::err2str(err) << '\n';
    }

    producer_->poll(0);
    producer_->flush(200);
}

void Kafka_Producer::flush() const {
    producer_->flush(5000);
}

