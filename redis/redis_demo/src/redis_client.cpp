//
// Created by Marat on 15.11.25.
//

#include "redis_client.h"

Redis_Client::Redis_Client(const std::string& host, int port)
    : port_(port)
    , host_(host)
    , context_(nullptr)
{
    connect();
}

bool Redis_Client::set(const std::string& key, const std::string& value) const {
    if (!context_) {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(context_, "SET %s %s", key.c_str(), value.c_str()));

    if (!reply) {
        return false;
    }

    freeReplyObject(reply);

    return true;
}

std::string Redis_Client::get(const std::string& key) const {
    if (!context_) {
        return {};
    }

    redisReply* reply = static_cast<redisReply *>(redisCommand(context_, "GET %s", key.c_str()));

    if (!reply) {
        return {};
    }

    std::string value = reply->type == REDIS_REPLY_STRING ? reply->str : "";
    freeReplyObject(reply);

    return value;
}

void Redis_Client::connect() {
    context_ = redisConnect(host_.c_str(), port_);
    if (!context_ || context_->err) {
        throw std::runtime_error("Failed to connect to Redis");
    }
}

Redis_Client::~Redis_Client() {
    if (context_) {
        redisFree(context_);
    }
}