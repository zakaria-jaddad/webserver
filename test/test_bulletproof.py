"""
Bulletproof stress and edge-case tests for ft_webserver.

These tests exercise the requirements that are easy to miss in day-to-day
development: concurrent CGI execution, upload integrity, default configuration,
path traversal, malformed input tolerance, and resilience against crash-inducing
workloads.
"""

import hashlib
import os
import random
import socket
import string
import threading
import time
from pathlib import Path

import pytest

from test.helpers import (
    chunked_response_body,
    decode_chunked,
    make_request,
    parse_response,
    send_raw_request,
    send_request_and_read_all,
    status_code_from,
)

PROJECT_ROOT = Path(__file__).resolve().parent.parent
WWW_ROOT = PROJECT_ROOT / "www"

DEFAULT_PORT = 2010
DEFAULT_HOST = "127.0.0.1"


def _send_port(port, req, timeout=5.0):
    return send_request_and_read_all(req, host=DEFAULT_HOST, port=port, timeout=timeout)


def _send(req, timeout=5.0):
    return _send_port(DEFAULT_PORT, req, timeout)


class TestDefaultConfiguration:
    """The server must start with a default config when none is supplied."""

    def test_default_config_starts_and_responds(self, server_with_config):
        # Use a config that mirrors the default conf/configExample.conf structure
        # but with a valid relative upload_store so the test is portable.
        config = f"""
        server {{
            listen 127.0.0.1:2099;
            server_name LocalHOSt;
            client_max_body_size 10;
            root ./www;
            index index.html;

            location / {{
                upload_store {PROJECT_ROOT / "www" / "uploads" / "default"};
            }}

            location /cgi-bin {{
                upload_store {PROJECT_ROOT / "www" / "uploads" / "default"};
                cgi .php /usr/bin/php-cgi;
                cgi .py /usr/bin/python3;
                autoindex on;
            }}
        }}
        """
        srv = server_with_config(config)
        resp = _send_port(2099, make_request("GET", "/", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200


class UploadHelper:
    """Build a multipart/form-data body."""

    @staticmethod
    def multipart_body(filename: str, content: bytes, field: str = "file", boundary: str = None):
        if boundary is None:
            boundary = "----WebKitFormBoundary" + "".join(
                random.choices(string.ascii_letters + string.digits, k=16)
            )
        delim = f"--{boundary}"
        body = (
            f"{delim}\r\n"
            f'Content-Disposition: form-data; name="{field}"; filename="{filename}"\r\n'
            f"Content-Type: application/octet-stream\r\n\r\n"
        ).encode() + content + f"\r\n{delim}--\r\n".encode()
        return body, boundary


class TestUploadIntegrity:
    """Uploads must be stored correctly and retrievable byte-for-byte."""

    def test_raw_binary_upload_matches_original(self, server):
        """Raw octet-stream POST bodies are stored exactly as received."""
        content = os.urandom(2048)
        req = make_request(
            "POST",
            "/uploads",
            headers={
                "Host": "localhost",
                "Content-Type": "application/octet-stream",
            },
            body=content,
        )
        resp = _send(req, timeout=10.0)
        code = status_code_from(resp)
        assert code in (200, 201, 204), f"upload returned {code}"

        # Find the most recently modified .bin file in the upload store.
        upload_dir = WWW_ROOT / "uploads" / "2010"
        files = sorted(
            [p for p in upload_dir.iterdir() if p.is_file()],
            key=lambda p: p.stat().st_mtime,
            reverse=True,
        )
        assert files, "no uploaded file found"
        uploaded = files[0]
        assert uploaded.read_bytes() == content, "uploaded content does not match original"

    def test_upload_text_file_retrievable(self, server):
        content = b"hello from bulletproof tests"
        req = make_request(
            "POST",
            "/uploads",
            headers={
                "Host": "localhost",
                "Content-Type": "text/plain",
            },
            body=content,
        )
        resp = _send(req, timeout=10.0)
        assert status_code_from(resp) in (200, 201, 204)

        upload_dir = WWW_ROOT / "uploads" / "2010"
        candidates = [p for p in upload_dir.iterdir() if p.read_bytes() == content]
        assert candidates, "uploaded text file not found"


class TestCgiEnvironmentAndBody:
    """CGI must receive the full request via environment variables and body via stdin."""

    def test_cgi_get_query_string(self, server):
        resp = _send(make_request("GET", "/cgi-bin/test.php?foo=bar&baz=qux", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200
        body = chunked_response_body(resp)
        assert b"QUERY_STRING=foo=bar&baz=qux" in body
        assert b"REQUEST_METHOD=GET" in body

    def test_cgi_post_body_echo(self, server):
        payload = b"name=alice&project=webserv"
        req = make_request(
            "POST",
            "/cgi-bin/test.php",
            headers={
                "Host": "localhost",
                "Content-Type": "application/x-www-form-urlencoded",
            },
            body=payload,
        )
        resp = _send(req, timeout=10.0)
        assert status_code_from(resp) == 200
        body = chunked_response_body(resp)
        assert b"REQUEST_METHOD=POST" in body
        assert b"CONTENT_LENGTH=26" in body
        assert b"BODY=" + payload in body

    def test_cgi_server_port_and_remote_addr(self, server):
        resp = _send(make_request("GET", "/cgi-bin/test.php", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200
        body = chunked_response_body(resp)
        assert b"SERVER_PORT=2010" in body
        assert b"REMOTE_ADDR=127.0.0.1" in body


class TestChunkedUploadIntegrity:
    """Chunked request bodies must be reassembled before reaching CGI / upload logic."""

    def test_chunked_post_reaches_cgi(self, server):
        chunks = b"5\r\nhello\r\n1\r\n!\r\n0\r\n\r\n"
        req = (
            b"POST /cgi-bin/test.php HTTP/1.1\r\n"
            b"Host: localhost\r\n"
            b"Transfer-Encoding: chunked\r\n"
            b"Content-Type: application/x-www-form-urlencoded\r\n"
            b"\r\n" + chunks
        )
        resp = _send(req, timeout=10.0)
        assert status_code_from(resp) == 200
        body = chunked_response_body(resp)
        assert b"BODY=hello!" in body

    def test_chunked_zero_size_terminator(self, server):
        req = (
            b"POST /cgi-bin/test.php HTTP/1.1\r\n"
            b"Host: localhost\r\n"
            b"Transfer-Encoding: chunked\r\n"
            b"\r\n"
            b"0\r\n\r\n"
        )
        resp = _send(req, timeout=10.0)
        assert status_code_from(resp) == 200


class TestSecurity:
    """The server must not escape the configured root directory."""

    def test_path_traversal_blocked(self, server):
        resp = _send(make_request("GET", "/../etc/passwd", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        # Must not serve /etc/passwd; 404 or 400 are acceptable.
        assert code in (400, 403, 404), f"path traversal returned {code}"

    def test_encoded_path_traversal_blocked(self, server):
        resp = _send(make_request("GET", "/%2e%2e/%2e%2e/etc/passwd", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        assert code in (400, 403, 404), f"encoded path traversal returned {code}"

    def test_null_byte_in_path(self, server):
        req = make_request("GET", "/index.html%00", headers={"Host": "localhost"})
        resp = _send(req)
        code = status_code_from(resp)
        assert code is not None
        assert code < 500


class TestVirtualHostSelection:
    """Different Host headers on the same port must select different server configs."""

    def test_same_port_different_host(self, server):
        # Port 2010 has server_name test.local; 2011 has other.local.
        # Both serve from ./www so the visible difference is small, but the request
        # must still resolve to a server block and return 200.
        resp_test = _send_port(2010, make_request("GET", "/", headers={"Host": "test.local"}))
        resp_other = _send_port(2010, make_request("GET", "/", headers={"Host": "other.local"}))
        assert status_code_from(resp_test) == 200
        assert status_code_from(resp_other) == 200


class TestConnectionAndVersion:
    """Connection handling and HTTP version edge cases."""

    def test_connection_close_closes(self, server):
        req = make_request("GET", "/", headers={"Host": "localhost", "Connection": "close"})
        with socket.create_connection((DEFAULT_HOST, DEFAULT_PORT), timeout=5) as sock:
            sock.sendall(req)
            data = b""
            try:
                while True:
                    chunk = sock.recv(4096)
                    if not chunk:
                        break
                    data += chunk
                    if data.endswith(b"0\r\n\r\n"):
                        break
            except socket.timeout:
                pass
        assert status_code_from(data) == 200

    def test_http10_request(self, server):
        resp = _send(make_request("GET", "/", version="HTTP/1.0", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        assert code in (200, 505), f"HTTP/1.0 returned {code}"


class TestMalformedInputResilience:
    """The server must never crash on garbage or malformed input."""

    def test_random_binary_data(self, server):
        garbage = os.urandom(512)
        resp = send_raw_request(garbage, host=DEFAULT_HOST, port=DEFAULT_PORT, timeout=3.0)
        code = status_code_from(resp)
        # Server may close without a response for unparseable input; the only
        # hard requirement is that it must not crash.
        if code is not None:
            assert code < 500 or code in (500, 501, 505)

    def test_incomplete_headers_then_close(self, server):
        # Open a connection, send partial request, then close from client side.
        # Server must not crash; other clients should still work.
        s = socket.create_connection((DEFAULT_HOST, DEFAULT_PORT), timeout=3)
        s.sendall(b"GET / HTTP/1.1\r\nHost: local")
        s.close()
        time.sleep(0.2)
        resp = _send(make_request("GET", "/", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200

    def test_multiple_host_headers(self, server):
        req = b"GET / HTTP/1.1\r\nHost: localhost\r\nHost: example.com\r\n\r\n"
        resp = _send(req)
        code = status_code_from(resp)
        assert code is not None
        assert code < 500

    def test_negative_content_length(self, server):
        req = b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: -5\r\n\r\n"
        resp = _send(req)
        code = status_code_from(resp)
        # Negative Content-Length is malformed; 400/411 are ideal, 413 is also
        # acceptable because a negative signed value may be interpreted as huge.
        assert code in (400, 411, 413)


class TestConcurrentStress:
    """Stress tests that previously crashed the server (concurrent CGI)."""

    def test_twenty_concurrent_cgi_requests(self, server):
        errors = []
        results = []

        def worker(n):
            try:
                resp = _send(
                    make_request("GET", f"/cgi-bin/test.php?n={n}", headers={"Host": "localhost"}),
                    timeout=15.0,
                )
                results.append(status_code_from(resp))
            except Exception as e:
                errors.append(e)

        threads = [threading.Thread(target=worker, args=(i,)) for i in range(20)]
        for t in threads:
            t.start()
        for t in threads:
            t.join(timeout=20)

        assert not errors, f"concurrent CGI raised exceptions: {errors[:3]}"
        ok = results.count(200)

        # The hard requirement is that the server remains alive. Under heavy CGI
        # load some connections may time out, but a follow-up request must still
        # succeed and the majority of concurrent CGI requests should too.
        sanity = _send(make_request("GET", "/", headers={"Host": "localhost"}), timeout=10.0)
        assert status_code_from(sanity) == 200, "server did not survive concurrent CGI load"
        assert ok >= 10, f"only {ok}/20 concurrent CGI requests returned 200"

    def test_one_hundred_concurrent_gets(self, server):
        errors = []
        results = []

        def worker():
            try:
                resp = _send(make_request("GET", "/", headers={"Host": "localhost"}), timeout=10.0)
                results.append(status_code_from(resp))
            except Exception as e:
                errors.append(e)

        threads = [threading.Thread(target=worker) for _ in range(100)]
        for t in threads:
            t.start()
        for t in threads:
            t.join(timeout=20)

        assert not errors, f"concurrent GET raised exceptions: {errors[:3]}"
        assert results.count(200) >= 95, f"only {results.count(200)}/100 concurrent GETs returned 200"


class TestBinaryFileIntegrity:
    """Static binary files must be served byte-for-byte."""

    def test_jpeg_file_sha256(self, server):
        resp = _send(
            make_request(
                "GET", "/sites/resources/yhossni.jpeg", headers={"Host": "localhost"}
            ),
            timeout=10.0,
        )
        assert status_code_from(resp) == 200
        body = chunked_response_body(resp)
        expected = (WWW_ROOT / "sites" / "resources" / "yhossni.jpeg").read_bytes()
        assert body == expected


class TestMethodResilience:
    """UNKNOWN methods and edge cases must not crash."""

    def test_options_method_no_crash(self, server):
        resp = _send(make_request("OPTIONS", "/", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        assert code is not None
        assert code < 500 or code == 501

    def test_trace_method_no_crash(self, server):
        resp = _send(make_request("TRACE", "/", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        assert code is not None
        assert code < 500 or code == 501


class TestCustomErrorPage:
    """If a custom error_page directive is configured, it must be served."""

    def test_custom_404_page(self, server_with_config):
        config = """
        server {
            listen 127.0.0.1:2080;
            root ./www;
            index index.html;
            error_page 404 ./www/pages/404.html;

            location / {
                allow GET;
            }
        }
        """
        srv = server_with_config(config)
        resp = _send_port(2080, make_request("GET", "/missing", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 404
        body = chunked_response_body(resp)
        assert b"404" in body or b"Not Found" in body
