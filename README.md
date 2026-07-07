# ft_webserver — 42 School Project

A non-blocking HTTP/1.1 web server written in **C++98**, inspired by NGINX. Uses a single `epoll` instance for all I/O multiplexing.

Build status: **119 automated tests pass** (including 24 bulletproof stress/edge-case tests).

---

## Features

### Mandatory

| Requirement | Status |
|---|---|
| Configuration file parsing (NGINX-like) | ✅ |
| Non-blocking I/O, single `epoll` loop | ✅ |
| Client disconnect handling & timeouts | ✅ |
| `GET`, `POST`, `DELETE` methods | ✅ |
| Static file serving (index, autoindex, MIME) | ✅ |
| File uploads with configurable storage | ✅ |
| CGI execution (`.php`, `.py` via `fork`+`execve`) | ✅ |
| CGI environment variables & chunked unchunking | ✅ |
| Multiple ports, different content per port | ✅ |
| Default error pages (200, 204, 400, 403, 404, 405, 408, 413, 500, 501, 504, 505) | ✅ |
| `client_max_body_size` enforcement | ✅ |
| Route-level method restrictions | ✅ |
| HTTP redirection per route | ✅ |
| Never crashes under load | ✅ |
| Compatible with standard browsers | ✅ |

### Bonus

| Feature | Status |
|---|---|
| HTTP cookies and session management | ✅ (via Python CGI `session.py`) |
| Multiple CGI types (PHP + Python) | ✅ |

---

## Requirements

- **C++98** standard, no external libraries, no Boost.
- Compiled with `c++ -Wall -Wextra -Werror -std=c++98`.
- Uses `epoll` (Linux) — single instance for all I/O.
- Every `read/recv/write/send` on sockets goes through `epoll_wait` first.
- Checking `errno` after `read/recv/write/send` is forbidden (exception: `send` in `ResponseCrafting.cpp` uses `EAGAIN`/`EWOULDBLOCK` which should be removed before evaluation).

---

## Compilation

```bash
make        # Build the executable
make clean  # Remove object files
make fclean # Remove object files and executable
make re     # Full rebuild
```

Produces the `ft_webserver` binary.

---

## Configuration

The server accepts a NGINX-inspired configuration file as its argument. If none is provided, it defaults to `conf/configExample.conf`.

### Directives

```nginx
server {
    listen 0.0.0.0:2010;
    server_name example.local;
    root ./www;
    index index.html;
    client_max_body_size 10m;

    location / {
        allow GET POST DELETE;
        upload_store /absolute/path/to/uploads;
        autoindex on;
        return 301 /new-path;
    }

    location /cgi-bin {
        cgi .php /usr/bin/php-cgi;
        cgi .py /usr/bin/python3;
        autoindex on;
    }
}
```

| Directive | Description |
|---|---|
| `listen` | `interface:port` pair (can appear multiple times in one server block) |
| `server_name` | Virtual host name (used for Host header matching) |
| `root` | Document root directory |
| `index` | Default file to serve for directories |
| `client_max_body_size` | Maximum client request body size (e.g. `10m`, `1k`) |
| `error_page` | Custom error page path |

**Location directives:**

| Directive | Description |
|---|---|
| `allow` | Space-separated list of accepted methods |
| `return` | HTTP redirect (`return 301 /target`) |
| `upload_store` | Directory for uploaded files (must be an absolute path) |
| `autoindex` | Enable/disable directory listing (`on`/`off`) |
| `cgi` | Map extension to CGI interpreter (`cgi .php /usr/bin/php-cgi`) |

### Test configuration

`conf/test.conf` is the primary test configuration and runs three virtual servers:

| Port | Server name | Notes |
|---|---|---|
| `2010` | `test.local` | Default site, full method access, uploads, CGI (PHP + Python) |
| `2011` | `other.local` | GET-only, demonstrates method restriction |
| `2012` | `redir.local` | Redirects `/old` → `/` |

---

## Execution

```bash
# With a configuration file
./ft_webserver conf/test.conf

# Default configuration
./ft_webserver
```

The server runs in the foreground. Press `Ctrl+C` to stop.

---

## Project structure

```
.
├── Makefile
├── ft_webserver               # Compiled binary
├── conf/
│   ├── configExample.conf     # Default configuration
│   └── test.conf              # Test suite configuration
├── includes/                  # Header files (.hpp)
├── src/
│   ├── ft_webserver.cpp       # Entry point
│   ├── cgi/                   # CGI execution and environment
│   ├── config/                # Configuration file parsing
│   ├── request/               # HTTP request parsing
│   ├── response/              # Response crafting, static methods, CGI execution
│   ├── server/                # Event loop, client management, epoll
│   └── utils/                 # Helpers (extension, method validation, server resolution)
├── test/                      # Automated test suite (pytest)
│   ├── conftest.py            # Pytest fixtures (server lifecycle)
│   ├── helpers.py             # Low-level HTTP helpers (raw sockets)
│   ├── test_bulletproof.py    # 24 stress/edge-case tests
│   ├── test_cgi.py            # CGI execution tests
│   ├── test_config.py         # Configuration parsing tests
│   ├── test_connections.py    # Connection management tests
│   ├── test_errors.py         # Error response tests
│   ├── test_extra_coverage.py # Virtual hosts, HEAD, timeouts, shutdown
│   ├── test_http_methods.py   # GET, POST, DELETE method tests
│   ├── test_pipeline.py       # HTTP/1.1 pipelining tests
│   ├── test_request_parsing.py# Malformed/edge-case request parsing tests
│   ├── test_smoke.py          # Sanity checks
│   ├── test_static_files.py   # Static file serving tests
│   ├── test_uploads.py        # File upload tests
│   └── manual_stress_test.sh  # Standalone curl/telnet manual test script
├── www/                       # Document root
│   ├── index.html             # Root index
│   ├── pages/                 # Default error pages
│   ├── cgi-bin/               # CGI scripts
│   │   ├── test.php           # Minimal test CGI (echoes env vars)
│   │   ├── equipment.php      # Equipment listing (PHP)
│   │   ├── session.py         # Session/cookies example (Python)
│   │   ├── song_request.py    # Song request form (Python)
│   │   └── backstage.py       # Backstage form (Python)
│   ├── sites/                 # Alternate site content
│   └── uploads/               # Upload storage
└── YoupiBanane/               # 42 tester directory (external)
```

---

## Testing

### Automated pytest suite (119 tests)

```bash
python3 -m pytest test/
```

Requires `python3` with `pytest` installed. Each test starts a fresh server process against `conf/test.conf`. Tests cover:

- **Static files**: existing files, 404, directory index, autoindex, MIME types, chunked encoding, keep-alive
- **HTTP methods**: GET, POST (URL-encoded, multipart), DELETE, method restrictions
- **CGI**: PHP and Python GET/POST, environment variables, query strings, POST body echo, CGI timeouts, non-existent scripts
- **Configuration**: multiple ports, virtual hosts, redirects, method restrictions, autoindex, default error pages, custom error pages
- **Request parsing**: malformed request lines, invalid versions, percent-encoding, oversized headers, chunked body, body size limits, duplicate headers, garbage bytes
- **Connections**: persistent connections, pipelining, concurrent clients, large URI, many headers, rapid sequential requests
- **Uploads**: text/binary file upload, missing boundary, upload without upload_store
- **Bulletproof**: raw upload integrity (byte-for-byte), CGI env var details, chunked upload reaching CGI, path traversal, 100 concurrent GETs, 20 concurrent CGIs, HTTP/1.0, `Connection: close`, random binary input, null byte, negative Content-Length, OPTIONS/TRACE no-crash

### Manual curl/telnet stress test

```bash
# Terminal 1 — start the server
./ft_webserver conf/test.conf

# Terminal 2 — run the stress script
./test/manual_stress_test.sh
```

The script tests every feature from the evaluation sheet: multiple ports, redirects, error pages, GET/POST/DELETE/UNKNOWN, MIME types, autoindex, upload/retrieve, CGI (PHP + Python), percent-encoding, body size limits, Connection: close, concurrent GETs (50), concurrent CGI (90), and raw telnet requests.

### Browser testing

Open `http://127.0.0.1:2010/` in any standard browser. Use the devtools Network tab to inspect request/response headers. Test the CGI forms and session example at the configured routes.

---

## Resources

- [RFC 7230 — HTTP/1.1: Message Syntax and Routing](https://tools.ietf.org/html/rfc7230)
- [RFC 7231 — HTTP/1.1: Semantics and Content](https://tools.ietf.org/html/rfc7231)
- [RFC 1945 — HTTP/1.0](https://tools.ietf.org/html/rfc1945)
- [RFC 3875 — CGI Version 1.1](https://tools.ietf.org/html/rfc3875)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [NGINX Documentation](https://nginx.org/en/docs/)
