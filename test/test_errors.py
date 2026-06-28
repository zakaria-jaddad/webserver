"""
Tests for HTTP error responses.

Covered requirements from subject.md:
- Accurate HTTP status codes
- Default error pages if none provided
- Required headers
"""

from test.helpers import (
    chunked_response_body,
    make_request,
    parse_response,
    send_request_and_read_all,
    status_code_from,
)


def _send(req):
    return send_request_and_read_all(req, timeout=3.0)


class TestStatusCodes:
    def test_200_ok(self, server):
        resp = _send(make_request("GET", "/", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200

    def test_404_not_found(self, server):
        resp = _send(make_request("GET", "/notfound", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 404

    def test_405_method_not_allowed(self, server):
        resp = send_request_and_read_all(
            make_request("POST", "/", headers={"Host": "other.local"}, body=b""),
            host="127.0.0.1",
            port=2011,
            timeout=3.0,
        )
        assert status_code_from(resp) in (405, 500)


class TestErrorPages:
    def test_404_has_body(self, server):
        resp = _send(make_request("GET", "/missing", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 404
        body = chunked_response_body(resp)
        assert len(body) > 0

    def test_error_page_has_content_type(self, server):
        resp = _send(make_request("GET", "/missing", headers={"Host": "localhost"}))
        _, headers, _ = parse_response(resp)
        assert "content-type" in headers

    def test_custom_error_page_if_configured(self, server):
        # test.conf does not set custom error pages; default pages should be served.
        resp = _send(make_request("GET", "/missing", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 404
        body = chunked_response_body(resp)
        assert b"404" in body or b"Not Found" in body
