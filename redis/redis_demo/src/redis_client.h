//
// Created by Marat on 15.11.25.
//

#pragma once

#include <string>
#include <hiredis/hiredis.h>

class Redis_Client {
public:
    explicit Redis_Client(const std::string& host, int port);

    bool set(const std::string& key, const std::string& value) const;
    std::string get(const std::string& key) const;

    ~Redis_Client();

private:
    void connect();

private:
    int port_;
    std::string host_;
    redisContext* context_;
};