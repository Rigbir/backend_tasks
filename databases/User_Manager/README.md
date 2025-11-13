# User Manager

User management system in C++ with PostgreSQL.

## Requirements

- C++20 compiler
- CMake 3.31+
- PostgreSQL
- libpqxx

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Setup

1. Create PostgreSQL database:
```sql
CREATE DATABASE UserManager;
```

2. Update connection string in `src/main.cpp`:
```cpp
Database db("dbname=UserManager user=your_user password=your_password");
```

## Project Structure

```
src/
├── database/
│   ├── Database.h
│   └── Database.cpp
├── service/
│   ├── UserService.h
│   └── UserService.cpp
└── main.cpp
```

## Architecture

- **Database** - database layer (CRUD operations)
- **UserService** - business logic for user management

## API

### UserService

- `registerUser(name, age)` - register user (duplicate check)
- `getAllUsers()` - get all users
- `getUsersByAgeRange(min, max)` - filter by age range
- `getUsersByName(name)` - search by name (partial match)
- `getUsersSorted(sortBy, ascending, limit, offset)` - sorted results with pagination
- `getUserCount()` - total user count
- `getAverageAge()` - average age
- `increaseUserAge(name, years)` - increase user age
- `deleteUserByName(name)` - delete user

## Run

```bash
./User_Manager
```

The program will execute a set of tests demonstrating all UserService methods.

## Features

- Optimized queries (no unnecessary data loading)
- Parameterized SQL queries (SQL injection protection)
- RAII and smart pointers
- Exception handling

