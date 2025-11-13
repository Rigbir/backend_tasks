//
// Created by Marat on 13.11.25.
//

#pragma once

#include "Database.h"

class UserService {
public:
    explicit UserService(Database& database);

    void registerUser(const std::string& name, int age) const;

    std::vector<Database::User> getAllUsers() const;
    std::vector<Database::User> getUsersByAgeRange(int minAge, int maxAge) const;
    std::vector<Database::User> getUsersByName(const std::string& name) const;
    std::vector<Database::User> getUsersSorted(
        const std::string& sortBy = "id",
        bool ascending = true,
        int limit = 0,
        int offset = 0
    ) const;
    int getUserCount() const;
    double getAverageAge() const;

    void increaseUserAge(const std::string& name, int years) const;
    void deleteUserByName(const std::string& name) const;

private:
    Database& database_;
};