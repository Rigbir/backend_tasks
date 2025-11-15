#include "Kafka_Producer.h"
#include "Kafka_Consumer.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    Kafka_Producer producer("localhost:9092", "test-topic");
    Kafka_Consumer consumer("localhost:9092", "my-group", "test-topic");

    std::thread cons([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "[CONSUMER] started, waiting for messages...\n";
        int message_count = 0;
        while (message_count < 5) {
            auto msg = consumer.poll_message();
            if (!msg.empty()) {
                std::cout << "[CONSUMER] " << msg << '\n';
                ++message_count;
            }
        }
        std::cout << "[CONSUMER] received all messages, exiting\n";
    });

    std::thread prod([&] {
        for (int i = 0; i < 5; ++i) {
            std::cout << "[PRODUCER] sending hello " + std::to_string(i) << '\n';
            producer.send("hello " + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        std::cout << "[PRODUCER] finished sending all messages\n";
    });

    prod.join();
    cons.join();
}