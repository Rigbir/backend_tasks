//
// Created by Marat on 13.11.25.
//

#include "UserService.h"

UserService::UserService(Database& database): database_(database) {}

void UserService::registerUser(const std::string& name, int age) const {
    if (!database_.userExists(name, age)) {
        database_.addUser(name, age);
    }
}

std::vector<Database::User> UserService::getAllUsers() const {
    return database_.getUsers();
}

std::vector<Database::User> UserService::getUsersByAgeRange(int minAge, int maxAge) const {
    return database_.getUserByAgeRange(minAge, maxAge);
}

std::vector<Database::User> UserService::getUsersByName(const std::string& name) const {
    return database_.findUsersByName(name);
}

std::vector<Database::User> UserService::getUsersSorted(const std::string& sortBy, bool ascending, int limit, int offset) const {
    return database_.getUsersSorted(sortBy, ascending, limit, offset);
}

int UserService::getUserCount() const {
    return database_.getUserCount();
}

double UserService::getAverageAge() const {
    return database_.getAverageAge();
}

void UserService::increaseUserAge(const std::string& name, int years) const {
    database_.increaseUserAge(name, years);
}

void UserService::deleteUserByName(const std::string& name) const {
    database_.deleteUser(name);
}