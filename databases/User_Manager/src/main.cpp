#include "UserService.h"
#include "Database.h"
#include <iostream>
#include <iomanip>

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void printUser(const Database::User& user) {
    std::cout << "  ID: " << std::setw(5) << user.id
              << " | Name: " << std::setw(15) << user.name
              << " | Age: " << std::setw(5) << user.age
              << " | Created: " << user.created_at << "\n";
}

void printUsers(const std::vector<Database::User>& users) {
    if (users.empty()) {
        std::cout << "  (no users found)\n";
        return;
    }
    for (const auto& user : users) {
        printUser(user);
    }
}

void testUserRegistration(UserService& userService) {
    printSeparator("TEST 1: User Registration");
    
    std::cout << "Registering users...\n";
    userService.registerUser("Alice", 25);
    userService.registerUser("Bob", 30);
    userService.registerUser("Charlie", 35);
    userService.registerUser("Alice", 25);  
    userService.registerUser("David", 28);
    userService.registerUser("Eve", 22);
    
    std::cout << "\nAll registered users:\n";
    printUsers(userService.getAllUsers());
}

void testGetAllUsers(UserService& userService) {
    printSeparator("TEST 2: Get All Users");
    
    auto users = userService.getAllUsers();
    std::cout << "Total users: " << users.size() << "\n";
    printUsers(users);
}

void testGetUsersByAgeRange(UserService& userService) {
    printSeparator("TEST 3: Get Users by Age Range");
    
    std::cout << "Users aged 25-30:\n";
    auto users = userService.getUsersByAgeRange(25, 30);
    printUsers(users);
    
    std::cout << "\nUsers aged 20-25:\n";
    users = userService.getUsersByAgeRange(20, 25);
    printUsers(users);
}

void testGetUsersByName(UserService& userService) {
    printSeparator("TEST 4: Get Users by Name (Partial Match)");
    
    std::cout << "Users with name containing 'Al':\n";
    auto users = userService.getUsersByName("Al");
    printUsers(users);
    
    std::cout << "\nUsers with name containing 'e':\n";
    users = userService.getUsersByName("e");
    printUsers(users);
}

void testGetUsersSorted(UserService& userService) {
    printSeparator("TEST 5: Get Users Sorted");
    
    std::cout << "Users sorted by age (ascending):\n";
    auto users = userService.getUsersSorted("age", true);
    printUsers(users);
    
    std::cout << "\nUsers sorted by name (descending), limit 3:\n";
    users = userService.getUsersSorted("name", false, 3);
    printUsers(users);
    
    std::cout << "\nUsers sorted by id (ascending), offset 2, limit 2:\n";
    users = userService.getUsersSorted("id", true, 2, 2);
    printUsers(users);
}

void testGetUserCount(UserService& userService) {
    printSeparator("TEST 6: Get User Count");
    
    int count = userService.getUserCount();
    std::cout << "Total number of users: " << count << "\n";
}

void testGetAverageAge(UserService& userService) {
    printSeparator("TEST 7: Get Average Age");
    
    double avgAge = userService.getAverageAge();
    std::cout << "Average age of all users: " << std::fixed 
              << std::setprecision(2) << avgAge << " years\n";
}

void testIncreaseUserAge(UserService& userService) {
    printSeparator("TEST 8: Increase User Age");
    
    std::cout << "Before age increase:\n";
    auto users = userService.getUsersByName("Alice");
    printUsers(users);
    
    std::cout << "\nIncreasing Alice's age by 5 years...\n";
    userService.increaseUserAge("Alice", 5);
    
    std::cout << "After age increase:\n";
    users = userService.getUsersByName("Alice");
    printUsers(users);
}

void testDeleteUser(UserService& userService) {
    printSeparator("TEST 9: Delete User");
    
    std::cout << "Before deletion:\n";
    printUsers(userService.getAllUsers());
    
    std::cout << "\nDeleting user 'Bob'...\n";
    userService.deleteUserByName("Bob");
    
    std::cout << "After deletion:\n";
    printUsers(userService.getAllUsers());
}

void testDuplicateRegistration(UserService& userService) {
    printSeparator("TEST 10: Duplicate Registration Prevention");
    
    std::cout << "Attempting to register duplicate (Alice, 30)...\n";
    int countBefore = userService.getUserCount();
    userService.registerUser("Alice", 30);
    int countAfter = userService.getUserCount();
    
    std::cout << "Users count before: " << countBefore 
              << ", after: " << countAfter << "\n";
    
    std::cout << "\nAttempting to register exact duplicate (Alice, 30)...\n";
    countBefore = countAfter;
    userService.registerUser("Alice", 30);
    countAfter = userService.getUserCount();
    
    std::cout << "Users count before: " << countBefore 
              << ", after: " << countAfter << "\n";
}

int main() {
    try {
        Database db("dbname=UserManager user=marat password=1111");
        db.createTable();
        
        db.clearUsers();
        
        UserService userService(db);
        
        testUserRegistration(userService);
        testGetAllUsers(userService);
        testGetUsersByAgeRange(userService);
        testGetUsersByName(userService);
        testGetUsersSorted(userService);
        testGetUserCount(userService);
        testGetAverageAge(userService);
        testIncreaseUserAge(userService);
        testDeleteUser(userService);
        testDuplicateRegistration(userService);
        
        printSeparator("ALL TESTS COMPLETED");
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}