# Swift Server

A multithreaded HTTP server in C++ using raw sockets. Supports basic routing, file handling, and concurrency.


## 📚 Table of Contents

- [Features](#features)
- [Build](#build)
- [Run](#run)
- [Supported Routes](#supported-routes)
- [Usage Examples](#usage-examples)


## Features

- `GET /echo/<message>` – returns `<message>`
- `GET /files/<filename>` – reads file from specified directory
- `POST /files/<filename>` – writes data to file
- Handles multiple clients via threads
- Returns proper HTTP status codes

## Build

```bash
g++ -std=c++17 -pthread server.cpp -o swift-server
```
## Run
```bash
./swift-server --directory ./files
```
If --directory is not provided, it uses a default path defined in the code.

## Supported Routes

| Method | Route                   | Description                          | Response Example                          |
|--------|-------------------------|--------------------------------------|-------------------------------------------|
| GET    | /echo/<message>         | Returns the message in response      | `hello` → `hello`                         |
| GET    | /files/<filename>       | Returns the contents of a file       | If `example.txt` has "Hi" → response: `Hi`|
| POST   | /files/<filename>       | Writes the body data to a file       | Responds with `HTTP/1.1 201 Created`      |
| ANY    | /<invalid>              | Handles unknown routes               | Responds with `HTTP/1.1 404 Not Found`    |

## Usage Examples

```bash

# Echo test
curl http://localhost:4221/echo/hello

# Get User-Agent
curl http://localhost:4221/user-agent

# Read file
curl http://localhost:4221/files/example.txt

# Write file
curl -X POST http://localhost:4221/files/new.txt --data "Hello, world!"
```

