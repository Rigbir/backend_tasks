#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include "Async_Kafka_Producer.h"
#include "Async_Kafka_Consumer.h"

int main() {
    const std::string brokers = "localhost:9092";
    const std::string topic = "test-topic-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    const std::string group_id = "test-group-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

    try {
        std::cout << "[1] Creating Producer first (to create topic)...\n";
        Async_Kafka_Producer producer(brokers, topic);
        std::cout << "    Producer created\n";
        
        std::cout << "[2] Sending a dummy message to create topic...\n";
        producer.send("dummy");
        producer.flush();
        std::cout << "    Topic created\n";
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        std::cout << "[3] Creating Consumer...\n";
        Async_Kafka_Consumer consumer(brokers, group_id, topic);
        std::cout << "    Consumer created successfully!\n\n";
        
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        std::atomic<int> messages_sent{0};
        std::atomic<int> messages_received{0};
        std::atomic<bool> producer_done{false};
        std::mutex cout_mutex;

        std::thread producer_thread([&]() {
            constexpr int total_messages = 10;
            
            for (int i = 1; i <= total_messages; ++i) {
                std::string message = "Message #" + std::to_string(i);
                producer.send(message);
                ++messages_sent;
                
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "[PRODUCER] Sent: " << message << "\n";
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            producer.flush();
            producer_done = true;
            
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "[PRODUCER] Finished sending " << total_messages << " messages\n";
            }
        });

        std::thread consumer_thread([&]() {
            constexpr int max_attempts = 200;
            int no_message_count = 0;
            bool dummy_received = false;
            
            for (int attempt = 0; attempt < max_attempts; ++attempt) {
                std::string msg = consumer.poll_message();
                
                if (!msg.empty()) {
                    if (msg == "dummy" && !dummy_received) {
                        dummy_received = true;
                        continue;
                    }
                    
                    no_message_count = 0;
                    ++messages_received;
                    {
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        std::cout << "[CONSUMER] Received [" << messages_received.load() 
                                  << "]: " << msg << "\n";
                    }
                } else {
                    ++no_message_count;
                }
                
                if (producer_done && messages_received >= messages_sent) {
                    break;
                }
                
                if (producer_done && no_message_count > 30) {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "[CONSUMER] Producer finished, no new messages for " 
                              << no_message_count << " attempts, stopping...\n";
                    break;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "[CONSUMER] Finished. Received " << messages_received.load() << " messages\n";
            }
        });

        std::cout << "\n[MAIN] Starting producer and consumer threads...\n";
        std::cout << "[MAIN] They will work ASYNCHRONOUSLY (in parallel)\n\n";

        producer_thread.join();
        consumer_thread.join();

        std::cout << "\n=== Test Results ===\n";
        std::cout << "Messages sent: " << messages_sent.load() << "\n";
        std::cout << "Messages received: " << messages_received.load() << "\n\n";

        std::cout << "[MAIN] Stopping Producer and Consumer...\n";
        producer.stop();
        consumer.stop();
        std::cout << "    Cleanup completed!\n\n";

        if (messages_received == messages_sent && messages_sent > 0) {
            std::cout << "SUCCESS: All messages sent and received asynchronously!\n";
            return 0;
        } else {
            std::cout << "WARNING: Some messages may be missing\n";
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    }
}