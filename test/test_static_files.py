"""
Tests for static file serving.

Covered requirements from subject.md:
- Serve a fully static website
- Existing files, missing files, directory handling
- Index files, autoindex, MIME types
- Required headers and Content-Length/Transfer-Encoding
"""

import pytest
from pathlib import Path

from test.helpers import (
    chunked_response_body,
    decode_chunked,
    make_request,
    parse_response,
    send_request_and_read_all,
    status_code_from,
)

PROJECT_ROOT = Path(__file__).resolve().parent.parent
WWW_ROOT = PROJECT_ROOT / "www"


def _send(req):
    return send_request_and_read_all(req, timeout=3.0)


class TestExistingFiles:
    def test_get_index_html(self, server):
        resp = _send(make_request("GET", "/", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200
        body = chunked_response_body(resp)
        assert b"<!DOCTYPE html>" in body

    def test_get_html_file(self, server):
        resp = _send(make_request("GET", "/test_dashboard.html", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200
        _, headers, _ = parse_response(resp)
        assert headers.get("content-type") == "text/html"

    def test_get_jpeg_image(self, server):
        resp = _send(make_request("GET", "/yhossni.jpg", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200
        _, headers, _ = parse_response(resp)
        assert headers.get("content-type") in ("image/jpeg", "image/jpg")

    def test_response_body_matches_file(self, server):
        resp = _send(make_request("GET", "/index.html", headers={"Host": "localhost"}))
        body = chunked_response_body(resp)
        expected = (WWW_ROOT / "index.html").read_bytes()
        assert body == expected


class TestMissingFiles:
    def test_404_for_missing_file(self, server):
        resp = _send(make_request("GET", "/does_not_exist.html", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 404

    def test_404_body_present(self, server):
        resp = _send(make_request("GET", "/missing", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 404
        body = chunked_response_body(resp)
        assert len(body) > 0


class TestDirectoryHandling:
    def test_directory_with_index(self, server):
        # / is a directory and has index.html.
        resp = _send(make_request("GET", "/", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 200
        body = chunked_response_body(resp)
        assert b"<!DOCTYPE html>" in body

    def test_directory_autoindex_enabled(self, server):
        # /folder has autoindex on in test.conf.
        resp = _send(make_request("GET", "/folder/", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        body = chunked_response_body(resp)
        # Either serves an index file or generates a directory listing.
        assert code in (200, 403)
        if code == 200:
            assert b"Index of" in body or b"<!DOCTYPE html>" in body

    def test_directory_without_index_or_autoindex(self, server):
        # /sites has no autoindex and index test.html.
        resp = _send(make_request("GET", "/sites/", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        assert code in (200, 403)


class TestMimeTypes:
    def test_html_mime(self, server):
        resp = _send(make_request("GET", "/index.html", headers={"Host": "localhost"}))
        _, headers, _ = parse_response(resp)
        assert headers.get("content-type") == "text/html"

    def test_jpeg_mime(self, server):
        resp = _send(make_request("GET", "/yhossni.jpg", headers={"Host": "localhost"}))
        _, headers, _ = parse_response(resp)
        assert headers.get("content-type") in ("image/jpeg", "image/jpg")

    def test_unknown_extension_default(self, server):
        # Create a file with unknown extension to verify default MIME handling.
        tmp_file = WWW_ROOT / "unknown.xyz"
        tmp_file.write_text("hello")
        try:
            resp = _send(make_request("GET", "/unknown.xyz", headers={"Host": "localhost"}))
            assert status_code_from(resp) == 200
            _, headers, _ = parse_response(resp)
            # Server should return some content-type.
            assert "content-type" in headers
        finally:
            tmp_file.unlink()


class TestResponseHeaders:
    def test_http11_chunked_encoding(self, server):
        resp = _send(make_request("GET", "/", headers={"Host": "localhost"}))
        _, headers, body = parse_response(resp)
        assert status_code_from(resp) == 200
        assert headers.get("transfer-encoding") == "chunked"

    def test_connection_keepalive_default(self, server):
        resp = _send(make_request("GET", "/", headers={"Host": "localhost"}))
        _, headers, _ = parse_response(resp)
        # HTTP/1.1 default is keep-alive unless Connection: close is sent.
        assert headers.get("connection", "keep-alive").lower() != "close"
