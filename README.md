# ft_webserver — HTTP/1.1 Web Server

![Language](https://img.shields.io/badge/Language-C++-00599C?style=flat&logo=c%2B%2B&logoColor=white)
![School](https://img.shields.io/badge/School-42-000000?style=flat)
![Protocol](https://img.shields.io/badge/Protocol-HTTP%2F1.1-blue?style=flat)
![Status](https://img.shields.io/badge/Status-Completed-success?style=flat)

**ft_webserver** is a custom HTTP/1.1 web server written in C++98 as part of the 42 School curriculum. It listens on configured IP/port pairs, accepts persistent TCP connections, parses HTTP requests, and serves static files, directory listings, redirects, file uploads, and CGI scripts.

---

## Overview

The server is built around a **single-threaded, non-blocking event loop** using Linux `epoll`. It handles many concurrent connections on one thread by switching between `EPOLLIN` and `EPOLLOUT` states per client. Request parsing is implemented as a state machine, and responses are typically sent using chunked transfer encoding.

---

## Features

- **HTTP Methods:** `GET`, `POST`, `DELETE`
- **Static File Serving** with root/index resolution
- **Directory Autoindex** (`autoindex on`) generating HTML listings
- **Custom Error Pages** for common status codes
- **Redirects** via `return <code> <path>`
- **Client Max Body Size** enforcement
- **File Uploads** via `multipart/form-data`
- **Chunked Request & Response Bodies**
- **CGI Execution** for `.php` scripts (also supports `.py` configuration)
- **Virtual Server Matching** by IP/port and `server_name`
- **Location Matching** by longest prefix
- **Connection Timeouts** for request and idle states
- **GoogleTest Unit Tests** for the request parser

---

## Approach & Architecture

| Component | Responsibility |
| :--- | :--- |
| **Server / Event Loop** | `epoll_create1`, `epoll_ctl`, `epoll_wait`; accepts clients and dispatches read/write events |
| **Configuration** | Nginx-inspired config tokenizer parsed into server and location blocks |
| **Request Parser** | Three-phase state machine: request line → headers → body |
| **Response Handler** | Routes methods, builds chunked responses, serves files, handles errors and CGI |
| **CGI** | `fork`/`execve` child process with pipes for STDIN/STDOUT and CGI/1.1 environment variables |

### Key Design Decisions
- **Non-blocking I/O:** all sockets are non-blocking so a single thread can manage many connections.
- **Edge-triggered `epoll`:** client state is tracked between `EPOLLIN` and `EPOLLOUT` events.
- **Chunked responses:** almost all responses are streamed in chunks for compatibility with keep-alive.
- **RAII-style cleanup:** client structs own their parser and response objects, which are freed on disconnect.

---

## Build & Run

> **Note:** This project uses Linux-specific headers (`<sys/epoll.h>`) and is intended to run on Linux.
>
> **Current working tree caveat:** `src/request/Request.cpp` is empty in the working tree. The full implementation is present in git history and can be restored with `git checkout src/request/Request.cpp`.

### Prerequisites
- Linux environment
- `g++` / `c++` with C++98 support
- `php-cgi` installed for PHP CGI support
- GoogleTest submodule initialized for tests

### Compilation
```bash
make
```

### Run
```bash
./ft_webserver                  # uses conf/configExample.conf
./ft_webserver myconfig.conf    # uses custom config
```

### Run Tests
```bash
make test
./ft_webserver_test
```

### Makefile Targets
- `make` — build the server
- `make test` — build and run unit tests
- `make clean` — remove object files
- `make fclean` — remove object files and binaries
- `make re` — rebuild from scratch

---

## Example Configuration

```nginx
server {
    listen 0.0.0.0:8080;
    server_name localhost;
    client_max_body_size 1048576;

    root ./www;
    index index.html;

    location / {
        upload_store /path/to/uploads;
    }

    location /php {
        cgi .php /usr/bin/php-cgi;
        autoindex on;
    }

    location /folder {
        autoindex on;
    }

    location /takeme-home {
        return 301 /index.html;
    }
}
```

---

## What I Learned

- Low-level Berkeley sockets and the full TCP server lifecycle.
- Scalable I/O multiplexing with `epoll` compared to `select`/`poll`.
- Designing non-blocking, state-machine-based protocol parsers.
- HTTP/1.1 message syntax: request lines, headers, chunked encoding, and multipart bodies.
- CGI/1.1 environment variables and process-spawning mechanics.
- Config-file tokenizer/parser design in C++98.
- Memory and resource management in a long-running C++ server.
- Practical debugging with AddressSanitizer and unit testing.

---

## Author

- [Zakaria Jaddad](https://github.com/zakaria-jaddad)
