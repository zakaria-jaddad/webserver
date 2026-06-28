"""
Tests for file upload handling.

Covered requirements from subject.md:
- Clients must be able to upload files
- Upload storage location configured per location
"""

import time
from pathlib import Path

from test.helpers import (
    chunked_response_body,
    make_request,
    send_request_and_read_all,
    status_code_from,
)


def _send(req):
    return send_request_and_read_all(req, timeout=5.0)


class TestFileUploads:
    def test_upload_text_file(self, server):
        boundary = "----WebKitFormBoundaryT1"
        content = b"Hello, webserv upload!"
        body = (
            f"------{boundary}\r\n"
            f"Content-Disposition: form-data; name=\"file\"; filename=\"uploaded.txt\"\r\n"
            f"Content-Type: text/plain\r\n\r\n"
        ).encode() + content + f"\r\n------{boundary}--\r\n".encode()

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
        assert code in (200, 201, 204), f"upload returned {code}"

    def test_upload_binary_file(self, server):
        boundary = "----WebKitFormBoundaryB2"
        content = bytes(range(256))
        body = (
            f"------{boundary}\r\n"
            f"Content-Disposition: form-data; name=\"bin\"; filename=\"bytes.bin\"\r\n"
            f"Content-Type: application/octet-stream\r\n\r\n"
        ).encode() + content + f"\r\n------{boundary}--\r\n".encode()

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
        assert code in (200, 201, 204), f"binary upload returned {code}"

    def test_upload_missing_boundary(self, server):
        req = make_request(
            "POST",
            "/uploads",
            headers={"Host": "localhost", "Content-Type": "multipart/form-data"},
            body=b"random body",
        )
        resp = _send(req)
        assert status_code_from(resp) in (400, 411)

    def test_post_without_upload_store(self, server):
        # Port 2011 has no upload_store configured.
        req = make_request(
            "POST",
            "/",
            headers={
                "Host": "other.local",
                "Content-Type": "application/x-www-form-urlencoded",
            },
            body=b"name=value",
        )
        resp = send_request_and_read_all(req, host="127.0.0.1", port=2011, timeout=3.0)
        assert status_code_from(resp) in (405, 500)
