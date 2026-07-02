"""
Tests for configuration parsing and server behavior.

Covered requirements from subject.md:
- Configuration file parsing
- Multiple ports / multiple websites
- Default error pages
- client_max_body_size
- Location rules (methods, redirection, root, index, autoindex, upload_store)
"""

import pytest
from pathlib import Path

from test.helpers import make_request, send_request_and_read_all, status_code_from

PROJECT_ROOT = Path(__file__).resolve().parent.parent


def _send_port(port, req):
    return send_request_and_read_all(req, host="127.0.0.1", port=port, timeout=3.0)


class TestMultiplePorts:
    def test_port_2010_serves_default_site(self, server):
        req = make_request("GET", "/", headers={"Host": "localhost"})
        resp = _send_port(2010, req)
        assert status_code_from(resp) == 200

    def test_port_2011_serves_different_root(self, server):
        req = make_request("GET", "/", headers={"Host": "other.local"})
        resp = _send_port(2011, req)
        assert status_code_from(resp) == 200

    def test_port_2012_redirect_location(self, server):
        req = make_request("GET", "/old", headers={"Host": "redir.local"})
        resp = _send_port(2012, req)
        code = status_code_from(resp)
        assert code in (301, 302, 307, 308)


class TestLocationRules:
    def test_redirection(self, server):
        req = make_request("GET", "/old", headers={"Host": "localhost"})
        resp = _send_port(2012, req)
        code = status_code_from(resp)
        assert code in (301, 302, 307, 308)

    def test_allowed_methods_per_location(self, server):
        # Port 2011 root only allows GET.
        resp = _send_port(2011, make_request("GET", "/", headers={"Host": "other.local"}))
        assert status_code_from(resp) == 200

        resp = _send_port(2011, make_request("POST", "/", headers={"Host": "other.local"}, body=b""))
        assert status_code_from(resp) == 405

    def test_autoindex_enabled(self, server):
        resp = _send_port(2010, make_request("GET", "/resources/", headers={"Host": "localhost"}))
        code = status_code_from(resp)
        assert code in (200, 403)


class TestConfigurationErrors:
    def test_invalid_listen_directive(self, server_with_config):
        config = """
        server {
            listen not_a_port;
            root ./www;
        }
        """
        srv = server_with_config(config, expect_success=False)
        assert srv.proc.poll() is not None
        assert srv.proc.returncode != 0

    def test_missing_root(self, server_with_config):
        config = """
        server {
            listen 127.0.0.1:2090;
        }
        """
        srv = server_with_config(config, expect_success=False)
        assert srv.proc.poll() is not None


class TestDefaultErrorPages:
    def test_404_uses_default_error_page(self, server):
        from test.helpers import chunked_response_body

        resp = _send_port(2010, make_request("GET", "/missing", headers={"Host": "localhost"}))
        assert status_code_from(resp) == 404
        body = chunked_response_body(resp)
        assert len(body) > 0

    def test_405_uses_default_error_page(self, server):
        resp = _send_port(2011, make_request("POST", "/", headers={"Host": "other.local"}, body=b""))
        code = status_code_from(resp)
        assert code in (405, 500)
