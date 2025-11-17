//
// Created by Marat on 17.11.25.
//

#include "Async_Kafka_Producer.h"
#include <iostream>
#include <stdexcept>

Async_Kafka_Producer::Async_Kafka_Producer(const std::string& brokers, const std::string& topic)
    : stop_(false) {
    std::string err;

    auto conf_deleter = [](RdKafka::Conf* conf) { delete conf; };
    std::unique_ptr<RdKafka::Conf, decltype(conf_deleter)> conf(
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL),
        conf_deleter
    );

    if (conf->set("bootstrap.servers", brokers, err) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set bootstrap.servers: " + err);
    }
    conf->set("batch.size", "1", err);
    conf->set("linger.ms", "0", err);

    producer_.reset(RdKafka::Producer::create(conf.get(), err));
    if (!producer_) {
        throw std::runtime_error("Producer creation failed: " + err);
    }

    std::unique_ptr<RdKafka::Conf, decltype(conf_deleter)> tconf (
        RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC),
        conf_deleter
    );

    topic_.reset(RdKafka::Topic::create(producer_.get(), topic, tconf.get(), err));
    if (!topic_) {
        throw std::runtime_error("Topic creation failed: " + err);
    }

    worker_ = std::thread(&Async_Kafka_Producer::worker_loop, this);
}

void Async_Kafka_Producer::send(const std::string& payload) {
    if (stop_.load()) {
        throw std::runtime_error("Producer is stopped");
    }

    RdKafka::ErrorCode err = producer_->produce(
        topic_.get(),
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char *>(payload.c_str()),
        payload.size(),
        nullptr,
        nullptr
    );

    if (err != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Produce failed: " << RdKafka::err2str(err) << '\n';
    }

    Message msg;
    msg.payload = payload;
    msg.error_code = err;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(msg));
    }
    cv_.notify_one();
}

void Async_Kafka_Producer::worker_loop() {
    while (!stop_.load()) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(100),
                         [this](){ return !queue_.empty() || stop_.load(); });

            if (stop_.load() && queue_.empty()) {
                break;
            }

            while (!queue_.empty()) {
                Message msg = std::move(queue_.front());
                queue_.pop();

                if (msg.error_code != RdKafka::ERR_NO_ERROR) {
                    std::cerr << "Message delivery failed: " 
                              << RdKafka::err2str(msg.error_code) << '\n';
                }
            }
        }

        producer_->poll(0);
    }

    producer_->flush(5000);
}

void Async_Kafka_Producer::flush() const {
    if (producer_) {
        producer_->flush(5000);
    }
}

void Async_Kafka_Producer::stop() {
    if (worker_.joinable()) {
        stop_ = true;
        cv_.notify_one();
        worker_.join();
    }
}

Async_Kafka_Producer::~Async_Kafka_Producer() {
    stop();
}