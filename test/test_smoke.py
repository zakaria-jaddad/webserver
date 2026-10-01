"""Sanity check that the test harness can build, start and query the server."""

from test.helpers import make_request, parse_response, send_request_and_read_all, status_code_from


def test_server_responds_200(server):
    resp = send_request_and_read_all(
        make_request("GET", "/", headers={"Host": "localhost"}), timeout=3.0
    )
    assert status_code_from(resp) == 200, resp.decode("utf-8", errors="replace")


def test_server_has_required_headers(server):
    resp = send_request_and_read_all(
        make_request("GET", "/", headers={"Host": "localhost"}), timeout=3.0
    )
    status_line, headers, _ = parse_response(resp)
    assert b"200 OK" in status_line
    assert "content-type" in headers
    assert "transfer-encoding" in headers or "content-length" in headers
