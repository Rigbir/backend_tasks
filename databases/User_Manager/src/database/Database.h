//
// Created by Marat on 13.11.25.
//

#pragma once

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <memory>

class Database {
public:
    struct User {
        int id;
        std::string name;
        int age;
        std::string created_at;
    };

public:
    explicit Database(const std::string& db_conn_str);

    void createTable() const;

    void addUser(const std::string& name, int age) const;
    std::vector<User> getUsers() const;
    std::vector<User> getUserByAgeRange(int minAge, int maxAge) const;
    std::vector<User> findUsersByName(const std::string& name) const;
    std::vector<User> getUsersSorted(
        const std::string& sortBy = "id",
        bool ascending = true,
        int limit = 0,
        int offset = 0
    ) const;
    int getUserCount() const;
    double getAverageAge() const;

    void updateUserAge(const std::string& name, int newAge) const;
    void increaseUserAge(const std::string& name, int years) const;

    void deleteUser(const std::string& name);
    void clearUsers();

    bool userExists(const std::string& name) const;
    bool userExists(const std::string& name, int age) const;

private:
    std::string m_conn_str;
    mutable std::unique_ptr<pqxx::connection> m_conn;

    void connect() const;
    static std::vector<User> dataCollection(const pqxx::result& res);
};