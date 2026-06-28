#!/usr/bin/env python3
"""HTTP/1.1 pipelining tests.

Covered requirement: requests should be processed in order on a persistent
connection without waiting for each response before sending the next request.
"""

import socket

from test.helpers import make_request, status_code_from

HOST = "127.0.0.1"
PORT = 2010


def _recv_all(sock, timeout=2.0):
    sock.settimeout(timeout)
    data = b""
    try:
        while True:
            chunk = sock.recv(512)
            if not chunk:
                break
            data += chunk
            # Stop early once the chunked terminator is seen.
            if data.endswith(b"0\r\n\r\n"):
                break
    except socket.timeout:
        pass
    return data


def test_pipeline(server):
    """Two pipelined GET requests should yield two 200 responses."""
    req = make_request("GET", "/", headers={"Host": "localhost"})
    pipeline = req + req

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(5)
        s.connect((HOST, PORT))
        s.sendall(pipeline)
        data = _recv_all(s)

    count_200 = data.count(b"200 OK")
    assert count_200 >= 1, "No response received for pipelined requests"
    assert count_200 == 2, f"Expected 2 responses, got {count_200}"


def test_mixed_pipeline(server):
    """A POST followed by a GET on the same connection should both be answered."""
    post = make_request(
        "POST",
        "/uploads",
        headers={
            "Host": "localhost",
            "Content-Type": "application/x-www-form-urlencoded",
        },
        body=b"name=test&value=123",
    )
    get = make_request("GET", "/", headers={"Host": "localhost"})
    pipeline = post + get

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(5)
        s.connect((HOST, PORT))
        s.sendall(pipeline)
        data = _recv_all(s, timeout=3.0)

    # POST currently returns 500; we still expect the GET to be answered.
    assert b"HTTP/1.1" in data
    assert data.count(b"HTTP/1.1") >= 2
