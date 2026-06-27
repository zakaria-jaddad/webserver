#!/usr/bin/env python3
import socket
import sys

HOST = "localhost"
PORT = 2010

def test_pipeline():
    # Build two pipelined GET requests
    req1 = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
    req2 = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
    pipeline = req1 + req2

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(5)
        s.connect((HOST, PORT))
        s.sendall(pipeline.encode())

        # Receive all data with a small buffer to force multiple reads
        data = b""
        s.settimeout(2)
        try:
            while True:
                chunk = s.recv(512)
                if not chunk:
                    break
                data += chunk
        except socket.timeout:
            pass

    response_text = data.decode('utf-8', errors='replace')
    print(response_text)
    count_200 = response_text.count("200 OK")

    print(f"Sent {len(pipeline)} bytes ({2} requests)")
    print(f"Received {len(data)} bytes")
    print(f"'200 OK' count: {count_200}")

    if count_200 == 2:
        print("✅ PIPELINING WORKS")
        return 0
    elif count_200 == 1:
        print("❌ Only one response — pipelining broken")
        return 1
    else:
        print(f"❌ Unexpected: {count_200} responses")
        return 1

def test_mixed_pipeline():
    """Test GET + POST pipeline (if you have a POST endpoint)"""
    # Adjust body and Content-Length for your setup
    body = "name=test&value=123"
    post = (
        "POST / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        f"Content-Length: {len(body)}\r\n"
        "\r\n"
        f"{body}"
    )
    get = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
    pipeline = post + get
    print(len(body))

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(5)
        s.connect((HOST, PORT))
        s.sendall(pipeline.encode())

        data = b""
        s.settimeout(2)
        try:
            while True:
                chunk = s.recv(512)
                if not chunk:
                    break
                data += chunk
        except socket.timeout:
            pass

    response_text = data.decode('utf-8', errors='replace')
    print(f"\nMixed pipeline (POST+GET):")
    print(f"Received {len(data)} bytes")
    # Look for 200 or 201/204 for POST, then 200 for GET
    print(response_text[:500])

if __name__ == "__main__":
    test_mixed_pipeline()
    rc = test_pipeline()
    sys.exit(0)
