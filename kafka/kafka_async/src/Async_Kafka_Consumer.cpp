//
// Created by Marat on 17.11.25.
//

#include "Async_Kafka_Consumer.h"
#include <vector>
#include <iostream>
#include <stdexcept>

Async_Kafka_Consumer::Async_Kafka_Consumer(const std::string& brokers,
                                           const std::string& group_id, 
                                           const std::string& topics)
    : stop_(false) {
    std::string err;

    auto conf_deleter = [](RdKafka::Conf* conf) { delete conf; };
    std::unique_ptr<RdKafka::Conf, decltype(conf_deleter)> conf(
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL),
        conf_deleter
    );

    conf->set("bootstrap.servers", brokers, err);
    conf->set("group.id", group_id, err);
    conf->set("auto.offset.reset", "earliest", err);
    conf->set("enable.auto.commit", "true", err);
    conf->set("auto.commit.interval.ms", "1000", err);

    consumer_.reset(RdKafka::KafkaConsumer::create(conf.get(), err));
    if (!consumer_) {
        throw std::runtime_error("Consumer creation failed: " + err);
    }

    std::vector<std::string> topics_vec = {topics};
    RdKafka::ErrorCode err_sub = consumer_->subscribe(topics_vec);
    if (err_sub != RdKafka::ERR_NO_ERROR) {
        throw std::runtime_error("Subscribe failed: " + RdKafka::err2str(err_sub));
    }

    worker_ = std::thread(&Async_Kafka_Consumer::worker_loop, this);
}

std::string Async_Kafka_Consumer::poll_message() {
    if (stop_.load()) {
        throw std::runtime_error("Consumer is stopped");
    }

    std::unique_lock<std::mutex> lock(mutex_);
    if (cv_.wait_for(lock, std::chrono::milliseconds(100),
                 [this](){ return !queue_.empty() || stop_.load(); })) {
        if (!queue_.empty()) {
            std::string msg = std::move(queue_.front());
            queue_.pop();
            return msg;
        }
    }

    return {};
}

void Async_Kafka_Consumer::worker_loop() {
    auto msg_deleter = [](RdKafka::Message* m) { delete m; };

    while (!stop_.load()) {
        std::unique_ptr<RdKafka::Message, decltype(msg_deleter)> msg(consumer_->consume(1000), msg_deleter);

        if (msg->err() == RdKafka::ERR_NO_ERROR) {
            if (msg->payload() != nullptr && msg->len() > 0) {
                std::string payload(static_cast<const char*>(msg->payload()), msg->len());
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    queue_.push(std::move(payload));
                }
                cv_.notify_one();
            }
        } else if (msg->err() == RdKafka::ERR__TIMED_OUT) {
            continue;
        } else if (msg->err() == RdKafka::ERR__PARTITION_EOF) {
            continue;
        } else if (msg->err() != RdKafka::ERR_NO_ERROR) {
            std::cerr << "[CONSUMER WORKER ERROR] " << msg->errstr() << " (code: " << msg->err() << ")\n";
        }
    }
}

void Async_Kafka_Consumer::stop() {
    if (worker_.joinable()) {
        stop_ = true;
        cv_.notify_one();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        worker_.join();
    }
}

Async_Kafka_Consumer::~Async_Kafka_Consumer() {
    stop();
}