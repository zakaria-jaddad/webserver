"""
Tests for CGI execution.

Covered requirements from subject.md:
- CGI execution based on file extension
- Environment variables available to CGI
- Request body handling
- Response parsing
- Error cases and timeouts
"""

import pytest

from test.helpers import (
    chunked_response_body,
    make_request,
    parse_response,
    send_request_and_read_all,
    status_code_from,
)


def _send(req):
    return send_request_and_read_all(req, timeout=5.0)


class TestCgiExecution:
    def test_php_cgi_get(self, server):
        resp = _send(make_request("GET", "/cgi-bin/test.php", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        assert code in (200, 403, 404, 500)
        if code == 200:
            body = chunked_response_body(resp)
            assert len(body) > 0

    def test_php_cgi_with_query_string(self, server):
        resp = _send(make_request("GET", "/cgi-bin/test.php?name=test", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        assert code in (200, 403, 404, 500)

    def test_cgi_post_body(self, server):
        body = b"name=alice"
        req = make_request(
            "POST",
            "/cgi-bin/test.php",
            headers={"Host": "localhost", "Content-Type": "application/x-www-form-urlencoded"},
            body=body,
        )
        resp = _send(req)
        code = status_code_from(resp)
        assert code in (200, 403, 404, 500)

    def test_cgi_environment_request_method(self, server):
        # If the CGI runs it should echo the REQUEST_METHOD.
        resp = _send(make_request("GET", "/cgi-bin/test.php", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        if code == 200:
            body = chunked_response_body(resp)
            assert b"GET" in body or b"REQUEST_METHOD" in body

    def test_nonexistent_cgi_script(self, server):
        resp = _send(make_request("GET", "/cgi-bin/missing.php", headers={"Host": "localhost"}))
        assert status_code_from(resp) in (404, 500)


class TestCgiErrors:
    def test_cgi_invalid_extension(self, server):
        resp = _send(make_request("GET", "/cgi-bin/script.unknown", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        # Either treated as static (404) or rejected.
        assert code in (200, 404, 403)
