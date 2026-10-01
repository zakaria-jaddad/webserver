# ft_Webserver — 42 School Project

> This project has been created as part of the 42 curriculum.

## Description

**ft_webserver** is a project where you build your own HTTP/1.1 web server in **C++98**. This is the moment you finally understand why URLs start with `HTTP`.

The goal is to implement a fully functional, non-blocking HTTP server that can handle multiple simultaneous client connections using a single I/O multiplexing call (`poll()`, `select()`, `epoll()`, or `kqueue()`). You will serve static websites, handle file uploads, execute CGI scripts, and manage various HTTP methods — all while ensuring your server remains resilient and never crashes.

By completing this project, you will gain a deep understanding of:

- The HTTP protocol and its request/response cycle
- Socket programming and TCP/IP networking
- Non-blocking I/O and event-driven architecture
- CGI (Common Gateway Interface) execution
- Configuration-driven server behavior (inspired by NGINX)

---

## Table of Contents

- [Mandatory Part](#mandatory-part)
  - [Requirements](#requirements)
  - [For macOS Only](#for-macos-only)
  - [Configuration File](#configuration-file)
- [Bonus Part](#bonus-part)
- [Allowed Functions](#allowed-functions)
- [Compilation](#compilation)
- [Execution](#execution)
- [Testing](#testing)
- [Resources](#resources)

---

## Mandatory Part

### Program Name

`ft_webserver`

### Files to Submit

- `Makefile`
- `*.{h, hpp}` — header files
- `*.cpp` — source files
- `*.tpp`, `*.ipp` — template implementation files
- **Configuration files** — for testing and demonstration

### Makefile Rules

Your `Makefile` must contain at least:

- `$(NAME)` / `all` — compiles the executable
- `clean` — removes object files
- `fclean` — removes object files and the executable
- `re` — performs `fclean` then `all`

> **Note:** The Makefile must not perform unnecessary relinking.

### Compilation Flags

```bash
c++ -Wall -Wextra -Werror -std=c++98
```

- Code must comply with the **C++98 standard**.
- Leverage C++ features where possible (e.g., prefer `<cstring>` over `<string.h>`).
- C functions are allowed, but prefer C++ equivalents when available.
- **No external libraries or Boost libraries** are permitted.

---

### Requirements

#### 1. Configuration File

- Your program must accept a **configuration file** as a command-line argument.
- If no configuration file is provided, a **default path** must be available.
- You **cannot** `execve` another web server.

#### 2. Non-Blocking I/O

- Your server must remain **non-blocking at all times**.
- Use **only 1** `poll()` (or equivalent: `select()`, `epoll()`, `kqueue()`) for **all** I/O operations between clients and the server (listen socket included).
- The multiplexing call must monitor **both reading and writing simultaneously**.
- You must **never** perform `read`/`recv` or `write`/`send` without going through `poll()` (or equivalent) first.
- **Checking `errno` after read/write is strictly forbidden** to adjust server behavior.

> **Exception:** Regular disk files do not require readiness notifications. `read()`/`write()` on disk files are exempt from the `poll()` requirement. However, all I/O that can wait for data (sockets, pipes, FIFOs, etc.) must be non-blocking and driven by a single `poll()` (or equivalent).

#### 3. Client Handling

- Properly handle **client disconnections** when necessary.
- A request to your server should **never hang indefinitely**.
- The server must be **compatible with standard web browsers**.

#### 4. HTTP Compliance

- Your HTTP **response status codes must be accurate**.
- The server must have **default error pages** if none are provided in the configuration.
- You need to implement at least the following HTTP methods:
  - `GET`
  - `POST`
  - `DELETE`

#### 5. Static Content & File Uploads

- You must be able to serve a **fully static website**.
- Clients must be able to **upload files** to the server.

#### 6. CGI Execution

- You can only use `fork()` for **CGI execution** (e.g., PHP, Python, etc.).
- CGI must be executed based on **file extension** (e.g., `.php`).
- The CGI should run in the **correct directory** for relative path file access.
- Your server must support **at least one CGI type** (php-CGI, Python, etc.).

**CGI Specifics:**

- The **full request and arguments** provided by the client must be available to the CGI via environment variables.
- For **chunked requests**, your server needs to **un-chunk** them. The CGI will expect **EOF** as the end of the body.
- If no `content_length` is returned from the CGI, **EOF** will mark the end of the returned data.

#### 7. Multiple Ports

- Your server must be able to **listen on multiple ports** to deliver different content (see Configuration File section).

#### 8. Resilience

- **Stress test** your server to ensure it remains available at all times.
- Your server must **never crash** under any circumstances (even if it runs out of memory) or terminate unexpectedly. If this occurs, the project is considered non-functional and graded 0.

#### 9. Virtual Hosts (Optional)

- Virtual hosts are considered **out of scope** for the mandatory part, but you are **allowed** to implement them if you want.

---

### For macOS Only

Since macOS handles `write()` differently from other Unix-based OSes:

- You are allowed to use `fcntl()` **only** with the following flags:
  - `F_SETFL`
  - `O_NONBLOCK`
  - `FD_CLOEXEC`
- **Any other `fcntl()` flag is forbidden.**
- File descriptors must be in **non-blocking mode** to achieve behavior similar to other Unix OSes.

---

### Configuration File

You can take inspiration from the `server` section of the **NGINX** configuration file.

Your configuration file must allow you to:

#### 1. Server Listeners

- Define all `interface:port` pairs on which your server will listen.
- This enables serving **multiple websites** from a single program.

#### 2. Error Pages

- Set up **default error pages**.

#### 3. Client Body Size

- Set the **maximum allowed size** for client request bodies.

#### 4. Route/URL Configuration

For each route (no regex required), you must be able to configure:

| Feature               | Description                                                                                                                                                                             |
| --------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Accepted Methods**  | List of accepted HTTP methods for the route (e.g., GET, POST, DELETE)                                                                                                                   |
| **HTTP Redirection**  | Configure redirects for specific routes                                                                                                                                                 |
| **Root Directory**    | Directory where the requested file should be located. E.g., if URL `/kapouet` is rooted to `/tmp/www`, then URL `/kapouet/pouic/toto/pouet` will search for `/tmp/www/pouic/toto/pouet` |
| **Directory Listing** | Enable or disable automatic directory listing                                                                                                                                           |
| **Default File**      | Default file to serve when the requested resource is a directory (e.g., `index.html`)                                                                                                   |
| **File Upload**       | Authorize file uploads from clients and specify the **storage location**                                                                                                                |
| **CGI Execution**     | Execute CGI based on file extension (e.g., `.php`)                                                                                                                                      |

#### 5. Additional Rules

- You can include other configuration information (e.g., a server name for virtual hosts).
- You must provide **configuration files and default files** to test and demonstrate that every feature works during evaluation.

> **Tip:** If you have a question about a specific behavior, compare your program's behavior with **NGINX**.

---

## Bonus Part

The bonus part will **only be assessed if the mandatory part is fully completed without issues**. If any mandatory requirement is missing, the bonus part will **not** be evaluated.

### Bonus Features

1. **Cookies and Session Management**

   - Support HTTP cookies.
   - Implement session management.
   - Provide simple working examples.

2. **Multiple CGI Types**
   - Handle more than one type of CGI (e.g., both PHP and Python).

---

## Allowed Functions

All functionality must be implemented in **C++98**. You may use the following functions:

```
execve, pipe, strerror, gai_strerror, errno, dup, dup2, fork,
socketpair, htons, htonl, ntohs, ntohl, select, poll, epoll
(epoll_create, epoll_ctl, epoll_wait), kqueue (kqueue, kevent),
socket, accept, listen, send, recv, chdir, bind, connect,
getaddrinfo, freeaddrinfo, setsockopt, getsockname, getprotobyname,
fcntl, close, read, write, waitpid, kill, signal, access, stat,
open, opendir, readdir, closedir
```

> **Note:** `poll()` is mentioned in the subject, but you can use any equivalent function such as `select()`, `kqueue()`, or `epoll()`.

---

## Compilation

```bash
make        # Compile the project
make clean  # Remove object files
make fclean # Remove object files and executable
make re     # Recompile from scratch
```

---

## Execution

```bash
./ft_webserver [configuration file]
```

- If no configuration file is provided, the server should use a default configuration path.

---

## Testing

Before starting this project:

1. **Read the RFCs** defining the HTTP protocol (HTTP/1.0 is suggested as a reference point).
2. **Perform tests** with `telnet` and **NGINX** to understand expected behaviors.
3. **Stress test** your server with multiple concurrent connections.
4. **Test with different browsers** to ensure compatibility.
5. **Write automated tests** in a suitable language (Python, Golang, C, C++, etc.).

> **Note:** A small tester is provided by 42. Using it is not mandatory if everything works with your browser and tests, but it can help you find and fix bugs.

---

## Resources

### Official Documentation

- [RFC 7230 — HTTP/1.1: Message Syntax and Routing](https://tools.ietf.org/html/rfc7230)
- [RFC 7231 — HTTP/1.1: Semantics and Content](https://tools.ietf.org/html/rfc7231)
- [RFC 1945 — HTTP/1.0](https://tools.ietf.org/html/rfc1945)
- [RFC 3875 — The Common Gateway Interface (CGI) Version 1.1](https://tools.ietf.org/html/rfc3875)

### Useful References

- [NGINX Documentation](https://nginx.org/en/docs/)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [MDN Web Docs — HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)
- [C++ Reference — `<cstring>`](https://en.cppreference.com/w/cpp/header/cstring)

### Tools

- `telnet` — for manual HTTP request testing
- `curl` — for automated HTTP testing
- `siege` / `ab` (Apache Bench) — for stress testing

---

## General Rules (42 Curriculum)

- Your program must **not crash under any circumstances** (even if it runs out of memory) or terminate unexpectedly.
- Submit a `Makefile` that compiles your source files without unnecessary relinking.
- Compile with `c++` and the flags `-Wall -Wextra -Werror`.
- Code must comply with the **C++98 standard**.
- Prefer C++ versions of C functions when possible.
- **No external libraries or Boost libraries** are allowed.

---

## Submission and Peer-Evaluation

- Submit your assignment in your **Git repository** as usual.
- Only the content of your repository will be evaluated.
- Double-check the names of your files to ensure they are correct.
- During evaluation, a **brief modification** of the project may be requested to verify your understanding.
- Be prepared to make small updates (e.g., modify a function, adjust behavior, add a feature) within a few minutes.

---
