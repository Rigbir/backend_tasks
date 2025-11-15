#include <iostream>
#include "http_server.h"
#include "redis_client.h"

int main() {
    auto redis = std::make_shared<Redis_Client>("127.0.0.1", 6379);

    Http_Server server(8080, redis);
    server.run();
}