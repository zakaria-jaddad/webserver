"""
Tests for client connection management.

Covered requirements from subject.md:
- Non-blocking server, single poll/epoll loop
- Proper client disconnections
- Persistent connections
- Large requests
- Multiple simultaneous clients
- Stress testing
"""

import socket
import threading
import time

import pytest

from test.helpers import make_request, send_request_and_read_all, status_code_from


def _send(req, host="127.0.0.1", port=2010, timeout=3.0):
    return send_request_and_read_all(req, host=host, port=port, timeout=timeout)


class TestPersistentConnections:
    def test_multiple_requests_on_same_connection(self, server):
        req = make_request("GET", "/", headers={"Host": "localhost"})
        with socket.create_connection(("127.0.0.1", 2010), timeout=5) as sock:
            sock.settimeout(5)
            for _ in range(3):
                sock.sendall(req)
                data = b""
                try:
                    while True:
                        chunk = sock.recv(4096)
                        if not chunk:
                            break
                        data += chunk
                        # Stop once we see the chunked terminator.
                        if data.endswith(b"0\r\n\r\n"):
                            break
                except socket.timeout:
                    pass
                assert status_code_from(data) == 200


class TestPipelining:
    def test_pipelined_get_requests(self, server):
        req = make_request("GET", "/", headers={"Host": "localhost"})
        pipeline = req + req + req
        resp = _send(pipeline, timeout=3.0)
        assert resp.count(b"HTTP/1.1 200 OK") >= 1


class TestSimultaneousClients:
    def test_ten_concurrent_clients(self, server):
        results = []

        def worker():
            try:
                resp = _send(make_request("GET", "/", headers={"Host": "localhost"}), timeout=5.0)
                results.append(status_code_from(resp))
            except Exception as e:
                results.append(e)

        threads = [threading.Thread(target=worker) for _ in range(10)]
        for t in threads:
            t.start()
        for t in threads:
            t.join(timeout=10)

        assert len(results) == 10
        for r in results:
            assert r == 200, f"Concurrent client failed: {r}"


class TestLargeRequests:
    def test_large_get_uri(self, server):
        long_path = "/" + "a" * 8000
        resp = _send(make_request("GET", long_path, headers={"Host": "localhost"}), timeout=5.0)
        code = status_code_from(resp)
        # RFC prefers 414; the subject does not mandate a length limit.
        assert code in (200, 404, 414)

    def test_many_headers(self, server):
        headers = {"Host": "localhost"}
        for i in range(100):
            headers[f"X-Header-{i}"] = "value"
        req = make_request("GET", "/", headers=headers)
        resp = _send(req, timeout=5.0)
        code = status_code_from(resp)
        assert code is not None
        assert code < 500


class TestStress:
    def test_rapid_sequential_requests(self, server):
        req = make_request("GET", "/", headers={"Host": "localhost"})
        for _ in range(50):
            resp = _send(req, timeout=3.0)
            assert status_code_from(resp) == 200
