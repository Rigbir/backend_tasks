# Simple Table

Simple PostgreSQL database operations demo in C++ with libpqxx.

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
CREATE DATABASE testdb;
```

2. Update connection string in `main.cpp`:
```cpp
static pqxx::connection c{"dbname=testdb user=your_user password=your_password"};
```

## Project Structure

```
.
├── main.cpp      - main program with CRUD operations
├── schema.sql    - database schema
└── CMakeLists.txt
```

## Features

- Create table from SQL schema
- Insert users (CREATE)
- Read all users (READ)
- Update user age (UPDATE)
- Delete user by name (DELETE)
- Clear all users

## Run

```bash
./Simple_Table
```
