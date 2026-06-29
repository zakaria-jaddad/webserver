"""
Tests for supported HTTP methods.

Covered requirements from subject.md:
- GET, POST, and DELETE methods must be supported
- Method restrictions per location
- Accurate status codes
"""

import pytest
from pathlib import Path

from test.helpers import (
    chunked_response_body,
    make_request,
    send_request_and_read_all,
    status_code_from,
)

PROJECT_ROOT = Path(__file__).resolve().parent.parent
WWW_ROOT = PROJECT_ROOT / "www"


def _send(req):
    return send_request_and_read_all(req, timeout=3.0)

#
class TestGet:
    def test_get_existing_file(self, server):
        resp = _send(make_request("GET", "/index.html", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200

    def test_get_returns_body(self, server):
        resp = _send(make_request("GET", "/index.html", headers={"Host": "localhost"}))
        body = chunked_response_body(resp)
        expected = (WWW_ROOT / "index.html").read_bytes()
        assert body == expected


class TestPost:
    def test_post_urlencoded(self, server):
        body = b"name=alice&value=42"
        req = make_request(
            "POST",
            "/uploads",
            headers={"Host": "localhost", "Content-Type": "application/x-www-form-urlencoded"},
            body=body,
        )
        resp = _send(req)
        code = status_code_from(resp)
        # POST must be supported; 500 indicates a server bug.
        assert code in (200, 201, 204), f"POST returned {code}"

    def test_post_multipart_upload(self, server):
        boundary = "----WebKitFormBoundary7MA4YWxk"
        # Per RFC 2046 the delimiter is "--" + the boundary parameter value.
        # boundary already starts with dashes, so do NOT prepend extra dashes.
        delim = f"--{boundary}"
        body = (
            f"{delim}\r\n"
            f"Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
            f"Content-Type: text/plain\r\n\r\n"
            f"hello upload\r\n"
            f"{delim}--\r\n"
        ).encode()
        req = make_request(
            "POST",
            "/uploads",
            headers={
                "Host": "localhost",
                "Content-Type": f"multipart/form-data; boundary={boundary}",
            },
            body=body,
        )
        resp = _send(req)
        code = status_code_from(resp)
        assert code in (200, 201, 204), f"multipart POST returned {code}"

    def test_post_to_disallowed_location(self, server):
        # Port 2011 only allows GET.
        req = make_request(
            "POST",
            "/",
            headers={"Host": "localhost"},
            body=b"",
        )
        resp = send_request_and_read_all(req, host="127.0.0.1", port=2011, timeout=3.0)
        assert status_code_from(resp) == 405


class TestDelete:
    def test_delete_existing_file(self, server):
        # Create a file in the upload directory that is writable.
        target = WWW_ROOT / "deleteme.txt"
        target.write_text("delete me")
        try:
            resp = _send(make_request("DELETE", "/deleteme.txt", headers={"Host": "localhost"}))
            assert status_code_from(resp) in (200, 204)
            assert not target.exists()
        finally:
            if target.exists():
                target.unlink()

    def test_delete_missing_file(self, server):
        resp = _send(make_request("DELETE", "/does_not_exist.txt", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 404

    def test_delete_forbidden_location(self, server):
        # Port 2011 only allows GET.
        req = make_request("DELETE", "/", headers={"Host": "localhost"})
        resp = send_request_and_read_all(req, host="127.0.0.1", port=2011, timeout=3.0)
        assert status_code_from(resp) == 405
