"""
Low-level HTTP helpers used across the webserv test suite.

These helpers deliberately use raw sockets where possible so that malformed or
edge-case requests can be sent exactly as written, without urllib normalizing
them.
"""

import re
import socket
import time
from pathlib import Path
from typing import Optional

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 2010


def send_raw_request(
    data: bytes | str,
    host: str = DEFAULT_HOST,
    port: int = DEFAULT_PORT,
    timeout: float = 5.0,
    recv_until_close: bool = False,
) -> bytes:
    """
    Open a TCP connection, send *data*, and read the response.

    If *recv_until_close* is True the socket is read until the server closes the
    connection; otherwise reading stops after the first timeout.
    """
    if isinstance(data, str):
        data = data.encode("utf-8")

    with socket.create_connection((host, port), timeout=timeout) as sock:
        sock.settimeout(timeout)
        sock.sendall(data)

        if recv_until_close:
            chunks = []
            try:
                while True:
                    chunk = sock.recv(4096)
                    if not chunk:
                        break
                    chunks.append(chunk)
            except socket.timeout:
                pass
            return b"".join(chunks)

        try:
            return sock.recv(65536)
        except socket.timeout:
            return b""


def send_request_and_read_all(
    data: bytes | str,
    host: str = DEFAULT_HOST,
    port: int = DEFAULT_PORT,
    timeout: float = 5.0,
) -> bytes:
    """Convenience wrapper that reads the full HTTP response.

    For chunked responses reading stops once the terminating chunk is seen,
    avoiding the full socket timeout for every request.
    """
    if isinstance(data, str):
        data = data.encode("utf-8")

    with socket.create_connection((host, port), timeout=timeout) as sock:
        sock.settimeout(timeout)
        sock.sendall(data)
        chunks = []
        try:
            while True:
                chunk = sock.recv(4096)
                if not chunk:
                    break
                chunks.append(chunk)
                # Heuristic: chunked terminator signals end of body.
                if b"0\r\n\r\n" in b"".join(chunks)[-64:]:
                    break
        except socket.timeout:
            pass
        return b"".join(chunks)


def decode_chunked(body: bytes) -> bytes:
    """Decode an HTTP/1.1 chunked transfer coding body.

    Tolerates leading CRLFs, which some server responses emit before the
    first chunk.
    """
    out = bytearray()
    idx = 0
    # Skip any leading CRLFs (non-RFC, but required for this server).
    while idx < len(body) and body[idx : idx + 2] == b"\r\n":
        idx += 2
    while idx < len(body):
        # Find the end of the chunk-size line.
        crlf = body.find(b"\r\n", idx)
        if crlf == -1:
            break
        size_line = body[idx:crlf].decode("ascii", errors="replace")
        # Chunk extensions are separated from the size by ';'.
        size_str = size_line.split(";", 1)[0].strip()
        try:
            size = int(size_str, 16)
        except ValueError:
            break
        if size == 0:
            # Last-chunk may be followed by trailers and a final CRLF.
            return bytes(out)
        idx = crlf + 2
        out.extend(body[idx : idx + size])
        idx += size
        # Skip trailing CRLF of chunk data.
        if body[idx : idx + 2] == b"\r\n":
            idx += 2
    return bytes(out)


def parse_response(raw: bytes) -> tuple[bytes, dict[str, str], bytes]:
    """
    Parse a raw HTTP response into (status_line, headers, body).

    Headers are returned lower-cased for easy lookup. The body is returned as
    received; use decode_chunked() when Transfer-Encoding: chunked is present.
    """
    raw = bytes(raw)
    # Split headers from body; handle case where body is absent.
    header_end = raw.find(b"\r\n\r\n")
    if header_end == -1:
        return raw, {}, b""

    header_block = raw[:header_end].decode("utf-8", errors="replace")
    body = raw[header_end + 4 :]

    lines = header_block.split("\r\n")
    status_line = lines[0].encode("utf-8")

    headers: dict[str, str] = {}
    for line in lines[1:]:
        if ":" not in line:
            continue
        name, value = line.split(":", 1)
        headers[name.strip().lower()] = value.strip()

    return status_line, headers, body


def status_code_from(raw: bytes) -> Optional[int]:
    """Extract the integer status code from a raw HTTP response."""
    status_line, _, _ = parse_response(raw)
    m = re.match(rb"HTTP/\d\.\d\s+(\d+)", status_line)
    if m:
        return int(m.group(1))
    return None


def chunked_response_body(raw: bytes) -> bytes:
    """Parse a response and return the decoded chunked body, or raw body."""
    _, headers, body = parse_response(raw)
    if headers.get("transfer-encoding", "").lower() == "chunked":
        return decode_chunked(body)
    return body


def make_request(
    method: str,
    path: str,
    *,
    version: str = "HTTP/1.1",
    headers: Optional[dict[str, str]] = None,
    body: Optional[bytes] = None,
) -> bytes:
    """Build a well-formed HTTP request byte string."""
    lines = [f"{method} {path} {version}"]
    headers = headers or {}
    if body is not None and "content-length" not in {k.lower() for k in headers}:
        headers["Content-Length"] = str(len(body))
    for name, value in headers.items():
        lines.append(f"{name}: {value}")
    lines.append("")
    request = "\r\n".join(lines).encode("utf-8") + b"\r\n"
    if body:
        request += body
    return request


def file_sha256(path: Path) -> str:
    """Return the SHA-256 hex digest of a file."""
    import hashlib

    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()
