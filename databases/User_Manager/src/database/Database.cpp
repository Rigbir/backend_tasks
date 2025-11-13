//
// Created by Marat on 13.11.25.
//

#include "Database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>

Database::Database(const std::string& db_conn_str): m_conn_str(db_conn_str) {}

void Database::createTable() const {
    connect();

    pqxx::work txn{*m_conn};

    std::fstream file("../schema.sql");
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open schema.sql");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    txn.exec(buffer.str());
    txn.commit();
}

void Database::addUser(const std::string& name, int age) const {
    connect();

    pqxx::work txn{*m_conn};
    txn.exec_params("INSERT INTO users (name, age) VALUES ($1, $2)", name, age);
    txn.commit();
}

std::vector<Database::User> Database::getUsers() const {
    connect();

    pqxx::work txn(*m_conn);
    pqxx::result res = txn.exec("SELECT id, name, age, created_at FROM users ORDER BY id;");
    return dataCollection(res);
}

std::vector<Database::User> Database::getUserByAgeRange(int minAge, int maxAge) const {
    connect();

    pqxx::work txn{*m_conn};

    pqxx::result res = txn.exec_params(
        "SELECT id, name, age, created_at FROM users "
        "WHERE age BETWEEN $1 AND $2 ORDER BY age;",
        minAge, maxAge
    );

    return dataCollection(res);
}

std::vector<Database::User> Database::findUsersByName(const std::string& name) const {
    connect();

    pqxx::work txn{*m_conn};

    pqxx::result res = txn.exec_params(
        "SELECT id, name, age, created_at FROM users "
        "WHERE name ILIKE '%' || $1 || '%' ORDER BY name;",
        name
    );

    return dataCollection(res);
}

std::vector<Database::User> Database::getUsersSorted(const std::string& sortBy, bool ascending, int limit, int offset) const {
    connect();

    pqxx::work txn{*m_conn};

    const std::set<std::string> allowedColumns = {"id", "name", "age", "created_at"};
    std::string column = "id";
    if (allowedColumns.count(sortBy)) {
        column = sortBy;
    }

    std::string order = ascending ? "ASC" : "DESC";

    std::stringstream sql;
    sql << "SELECT id, name, age, created_at FROM users ORDER BY "
        << column << " " << order;

    if (limit > 0) {
        sql << " LIMIT " << limit;
    }
    if (offset > 0) {
        sql << " OFFSET " << offset;
    }
    sql << ";";

    pqxx::result res = txn.exec(sql.str());

    return dataCollection(res);
}

int Database::getUserCount() const {
    connect();

    pqxx::work txn{*m_conn};

    pqxx::result res = txn.exec("SELECT COUNT(*) FROM users;");

    int count = res[0][0].as<int>();
    return count;
}

double Database::getAverageAge() const {
    connect();
    pqxx::work txn{*m_conn};

    pqxx::result res = txn.exec("SELECT AVG(age) FROM users;");

    if (res[0][0].is_null()) {
        return 0.0;
    }

    double avg = res[0][0].as<double>();
    return avg;
}

void Database::updateUserAge(const std::string& name, int newAge) const {
    connect();

    pqxx::work txn{*m_conn};
    txn.exec_params("UPDATE users SET age = $1 WHERE name = $2;", newAge, name);
    txn.commit();
}

void Database::increaseUserAge(const std::string& name, int years) const {
    connect();

    pqxx::work txn{*m_conn};
    txn.exec_params("UPDATE users SET age = age + $1 WHERE name = $2;", years, name);
    txn.commit();
}

void Database::deleteUser(const std::string& name) {
    connect();

    pqxx::work txn{*m_conn};
    txn.exec_params("DELETE FROM users WHERE name = $1;", name);
    txn.commit();
}

void Database::clearUsers() {
    connect();

    pqxx::work txn{*m_conn};
    txn.exec("DELETE FROM users;");
    txn.commit();
}

bool Database::userExists(const std::string& name) const {
    connect();

    pqxx::work txn{*m_conn};
    pqxx::result res = txn.exec_params("SELECT 1 FROM users WHERE name = $1;", name);
    return !res.empty();
}

bool Database::userExists(const std::string& name, int age) const {
    connect();

    pqxx::work txn{*m_conn};
    pqxx::result res = txn.exec_params("SELECT 1 FROM users WHERE name = $1 AND age = $2;", name, age);
    return !res.empty();
}

void Database::connect() const {
    if (!m_conn || !m_conn->is_open()) {
        try {
            m_conn = std::make_unique<pqxx::connection>(m_conn_str);
        } catch (const pqxx::sql_error& e) {
            std::cerr << "SQL error: " << e.what() << "\nQuery: " << e.query() << '\n';
            throw; 
        } catch (const std::exception& e) {
            std::cerr << "Connection error: " << e.what() << '\n';
            throw;  
        }
    }
    
    if (!m_conn || !m_conn->is_open()) {
        throw std::runtime_error("Failed to establish database connection");
    }
}

std::vector<Database::User> Database::dataCollection(const pqxx::result& res) {
    std::vector<User> usersInfo;
    usersInfo.reserve(res.size());

    for (const auto& row : res) {
        User currentUser;

        currentUser.id =         row["id"].as<int>();
        currentUser.name =       row["name"].as<std::string>();
        currentUser.age =        row["age"].as<int>();
        currentUser.created_at = row["created_at"].as<std::string>();

        usersInfo.push_back(std::move(currentUser));
    }

    return usersInfo;
}