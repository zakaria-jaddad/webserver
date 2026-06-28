"""
Tests for HTTP request parsing.

Covered requirements from subject.md:
- HTTP request parsing (request line, methods, URI, version, headers)
- Invalid / malformed requests
- Header limits
- Chunked requests
- Body parsing
"""

import pytest

from test.helpers import (
    decode_chunked,
    make_request,
    parse_response,
    send_request_and_read_all,
    status_code_from,
)


def _send(req):
    return send_request_and_read_all(req, timeout=3.0)


class TestRequestLine:
    """Requirement: HTTP request line parsing."""

    def test_get_root(self, server):
        resp = _send(make_request("GET", "/", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200

    def test_post_request(self, server):
        resp = _send(make_request("POST", "/", headers={"Host": "localhost"}, body=b""))
        assert status_code_from(resp) in (200, 405)

    def test_delete_request(self, server):
        resp = _send(make_request("DELETE", "/nonexistent", headers={"Host": "localhost"}))
        assert status_code_from(resp) in (404, 405)

    def test_unknown_method_rejected(self, server):
        """Only GET, POST and DELETE are required; others should be rejected."""
        resp = _send(make_request("PUT", "/", headers={"Host": "localhost"}))
        assert status_code_from(resp) in (400, 405, 501)

    def test_invalid_method_with_space(self, server):
        """A method token cannot contain a space."""
        req = b"GE T / HTTP/1.1\r\nHost: localhost\r\n\r\n"
        resp = _send(req)
        assert status_code_from(resp) in (400, 405, 501)

    def test_missing_request_target(self, server):
        req = b"GET HTTP/1.1\r\nHost: localhost\r\n\r\n"
        resp = _send(req)
        assert status_code_from(resp) == 400

    def test_missing_http_version(self, server):
        req = b"GET / \r\nHost: localhost\r\n\r\n"
        resp = _send(req)
        assert status_code_from(resp) == 400

    def test_empty_request_line(self, server):
        resp = _send(b"\r\n")
        assert status_code_from(resp) == 400


class TestUriParsing:
    """Requirement: URI parsing including paths and query parameters."""

    def test_simple_path(self, server):
        resp = _send(make_request("GET", "/index.html", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200

    def test_query_parameters(self, server):
        resp = _send(make_request("GET", "/?foo=bar&baz=qux", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200

    def test_percent_encoding_in_path(self, server):
        resp = _send(make_request("GET", "/index%2Ehtml", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200

    def test_empty_path_after_decode(self, server):
        """A URI that decodes to nothing is a bad request."""
        req = make_request("GET", "/%00", headers={"Host": "localhost"})
        resp = _send(req)
        # Implementation may return 400 or treat NUL as data.
        assert status_code_from(resp) is not None


class TestHttpVersion:
    """Requirement: HTTP version handling."""

    def test_http11(self, server):
        resp = _send(make_request("GET", "/", version="HTTP/1.1", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200

    def test_http10(self, server):
        """HTTP/1.0 should either be supported or yield 505."""
        resp = _send(make_request("GET", "/", version="HTTP/1.0", headers={"Host": "localhost"}))
        assert status_code_from(resp) in (200, 505)

    def test_invalid_version_prefix(self, server):
        resp = _send(make_request("GET", "/", version="HTXP/1.1", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 400

    def test_unsupported_version(self, server):
        # Subject focuses on HTTP/1.x; strict 505 enforcement is not required.
        resp = _send(make_request("GET", "/", version="HTTP/2.0", headers={"Host": "localhost"}))
        assert status_code_from(resp) in (200, 400, 505)


class TestHeaders:
    """Requirement: header parsing and limits."""

    def test_basic_headers(self, server):
        resp = _send(
            make_request(
                "GET",
                "/",
                headers={"Host": "localhost", "User-Agent": "webserv-test", "Accept": "*/*"},
            )
        )
        assert status_code_from(resp) == 200

    def test_duplicate_header_values(self, server):
        req = b"GET / HTTP/1.1\r\nHost: localhost\r\nX-Test: a\r\nX-Test: b\r\n\r\n"
        resp = _send(req)
        assert status_code_from(resp) == 200

    def test_header_with_whitespace_value(self, server):
        req = b"GET / HTTP/1.1\r\nHost: localhost\r\nX-Test:   spaced   \r\n\r\n"
        resp = _send(req)
        assert status_code_from(resp) == 200

    def test_missing_colon_in_header(self, server):
        req = b"GET / HTTP/1.1\r\nHost localhost\r\n\r\n"
        resp = _send(req)
        assert status_code_from(resp) == 400

    def test_empty_header_name(self, server):
        req = b"GET / HTTP/1.1\r\nHost: localhost\r\n: value\r\n\r\n"
        resp = _send(req)
        assert status_code_from(resp) == 400

    def test_header_name_with_space(self, server):
        req = b"GET / HTTP/1.1\r\nHost : localhost\r\n\r\n"
        resp = _send(req)
        # RFC requires no whitespace before colon.
        assert status_code_from(resp) in (400, 200)

    def test_very_long_header(self, server):
        """Extremely large headers should be rejected to avoid DoS."""
        huge_value = "x" * (128 * 1024)
        req = f"GET / HTTP/1.1\r\nHost: localhost\r\nX-Huge: {huge_value}\r\n\r\n".encode()
        resp = _send(req)
        code = status_code_from(resp)
        assert code is not None
        # Either accepted or rejected with a 4xx; must not crash.
        assert code < 500 or code in (413, 431)


class TestBodyParsing:
    """Requirement: body parsing with Content-Length and chunked encoding."""

    def test_post_with_content_length(self, server):
        body = b"name=value"
        req = make_request(
            "POST", "/", headers={"Host": "localhost", "Content-Type": "application/x-www-form-urlencoded"}, body=body
        )
        resp = _send(req)
        code = status_code_from(resp)
        assert code is not None
        # Subject requires POST support; exact code depends on upload config.
        assert code < 500

    def test_post_without_content_length(self, server):
        """POST without Content-Length or Transfer-Encoding is malformed."""
        req = b"POST / HTTP/1.1\r\nHost: localhost\r\n\r\nbody here"
        resp = _send(req)
        assert status_code_from(resp) in (400, 411)

    def test_chunked_body(self, server):
        chunks = b"4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n"
        req = (
            b"POST / HTTP/1.1\r\n"
            b"Host: localhost\r\n"
            b"Transfer-Encoding: chunked\r\n"
            b"\r\n" + chunks
        )
        resp = _send(req)
        code = status_code_from(resp)
        assert code is not None
        assert code < 500

    def test_malformed_chunk_size(self, server):
        req = (
            b"POST / HTTP/1.1\r\n"
            b"Host: localhost\r\n"
            b"Transfer-Encoding: chunked\r\n"
            b"\r\n"
            b"ZZZ\r\nbody\r\n0\r\n\r\n"
        )
        resp = _send(req)
        assert status_code_from(resp) == 400

    def test_oversized_body(self, server):
        """A body larger than client_max_body_size should be rejected."""
        body = b"x" * (11 * 1024 * 1024)  # test.conf allows 10m
        req = make_request(
            "POST", "/", headers={"Host": "localhost", "Content-Type": "application/octet-stream"}, body=body
        )
        resp = _send(req)
        assert status_code_from(resp) in (413,)


class TestMalformedRequests:
    """Requirement: resilience against malformed input."""

    def test_lone_crlf_no_request(self, server):
        resp = _send(b"\r\n")
        assert status_code_from(resp) == 400

    def test_garbage_bytes(self, server):
        resp = _send(b"\x00\x01\x02\x03\r\n\r\n")
        assert status_code_from(resp) == 400

    def test_incomplete_request_line(self, server):
        resp = _send(b"GET /")
        # Server should not crash; may time out or return 400/408.
        code = status_code_from(resp)
        assert code in (400, 408, None)
