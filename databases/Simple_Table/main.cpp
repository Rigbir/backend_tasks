#include <pqxx/pqxx>
#include <iostream>
#include <fstream>
#include <sstream>

pqxx::connection& get_connection() {
    static pqxx::connection c{"dbname=testdb user=marat password=1234"};
    return c;
}

void create_table() {
    try {
        pqxx::work txn{get_connection()};

        std::ifstream file("../schema.sql");
        std::stringstream buffer;
        buffer << file.rdbuf();

        txn.exec(buffer.str());
        txn.commit();

        std::cout << "Schema applied successfully\n";
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

void create_request() {
    try {
        pqxx::work txn{get_connection()};

        txn.exec("INSERT INTO users (name, age) VALUES ('Alice', 25)");
        txn.exec("INSERT INTO users (name, age) VALUES ('Bob', 30)");
        txn.commit();

        std::cout << "Users inserted successfully\n";
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

void read_request() {
    try {
        pqxx::work txn{get_connection()};
        pqxx::result res = txn.exec("SELECT id, name, age, created_at FROM users ORDER BY id;");

        for (auto row : res) {
            std::cout << "ID: "           << row["id"].as<int>()
                      << ", Name: "       << row["name"].as<std::string>()
                      << ", Age: "        << row["age"].as<int>()
                      << ", Created at: " << row["created_at"].as<std::string>()
                      << '\n';
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

void update_UserAge(const std::string& name, int newAge) {
    try {
        pqxx::work txn{get_connection()};

        std::string sql = "UPDATE users SET age = " + std::to_string(newAge) +
                          " WHERE name = " + txn.quote(name) + ";";

        int rows = txn.exec(sql).affected_rows();
        txn.commit();

        std::cout << "Updated " << rows << " rows for user " << name << '\n';
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

void delete_UserByName(const std::string& name) {
    try {
        pqxx::work txn{get_connection()};

        std::string sql = "DELETE FROM users WHERE name = " + txn.quote(name) + ";";
        int rows = txn.exec(sql).affected_rows();

        txn.commit();
        std::cout << "Deleted " << rows << " rows for user " << name << '\n';
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

void clear() {
    try {
        pqxx::work txn{get_connection()};

        int rows = txn.exec("DELETE FROM users;").affected_rows();

        txn.commit();
        std::cout << "Deleted " << rows << " rows\n";
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

int main() {
    try {
        create_table();
        clear();

        create_request();
        read_request();

        update_UserAge("Bob", 15);
        read_request();

        delete_UserByName("Bob");
        read_request();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}