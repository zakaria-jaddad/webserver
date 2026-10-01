"""
Pytest fixtures and helpers for the webserv test suite.

All tests are derived from requirements in subject.md.
"""

import os
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

import pytest

PROJECT_ROOT = Path(__file__).resolve().parent.parent
SERVER_BINARY = PROJECT_ROOT / "ft_webserver"
DEFAULT_CONFIG = PROJECT_ROOT / "conf" / "test.conf"
TEST_UPLOAD_DIR = PROJECT_ROOT / "test_uploads"


def wait_for_server(host: str, port: int, timeout: float = 10.0) -> bool:
    """Wait until a TCP server is accepting connections."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with socket.create_connection((host, port), timeout=0.5):
                return True
        except (socket.timeout, ConnectionRefusedError, OSError):
            time.sleep(0.05)
    return False


def build_server() -> None:
    """Build the server using make if the binary is missing or source changed."""
    if SERVER_BINARY.exists():
        mtime_bin = SERVER_BINARY.stat().st_mtime
        needs_rebuild = False
        for src in (PROJECT_ROOT / "src").rglob("*"):
            if src.is_file() and src.stat().st_mtime > mtime_bin:
                needs_rebuild = True
                break
        if not needs_rebuild:
            return

    result = subprocess.run(
        ["make", "all"],
        cwd=PROJECT_ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"Failed to build server:\n{result.stdout}")


class ServerProcess:
    """Context manager around the webserv subprocess."""

    def __init__(self, config_path: Path, *, expect_success: bool = True, capture_output: bool = False):
        self.config_path = config_path
        self.expect_success = expect_success
        self.capture_output = capture_output
        self.proc = None
        self._output: list[str] = []
        self._drain_thread = None

    def _start_draining(self):
        if not self.capture_output or self.proc.stdout is None:
            return

        def drain():
            try:
                for line in self.proc.stdout:
                    self._output.append(line)
            except Exception:
                pass

        import threading

        self._drain_thread = threading.Thread(target=drain, daemon=True)
        self._drain_thread.start()

    def start(self) -> "ServerProcess":
        build_server()
        env = os.environ.copy()
        # Sanitize upload test directories before each run. Wipe stale uploads
        # but recreate every upload_store referenced by the config, since the
        # server does not create them itself and will return 500 for a POST to
        # a route whose upload_store is missing (Request.cpp _initParsingInfo).
        if TEST_UPLOAD_DIR.exists():
            shutil.rmtree(TEST_UPLOAD_DIR)
        TEST_UPLOAD_DIR.mkdir(parents=True, exist_ok=True)
        for store in read_config_upload_stores(self.config_path):
            store.mkdir(parents=True, exist_ok=True)

        stdout = subprocess.PIPE if self.capture_output else subprocess.DEVNULL
        self.proc = subprocess.Popen(
            [str(SERVER_BINARY), str(self.config_path)],
            cwd=PROJECT_ROOT,
            stdout=stdout,
            stderr=subprocess.STDOUT,
            text=True,
            env=env,
        )
        self._start_draining()
        # Give the process a moment to either fail fast or start listening.
        time.sleep(0.3)
        if self.proc.poll() is not None:
            output = self.stdout()
            if self.expect_success:
                raise RuntimeError(
                    f"Server exited early with code {self.proc.returncode}:\n{output}"
                )
        return self

    def stdout(self) -> str:
        if self.capture_output:
            return "".join(self._output)
        return ""

    def stop(self) -> str:
        if self.proc is None:
            return ""
        try:
            self.proc.terminate()
            self.proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            self.proc.wait(timeout=3)
        if self.capture_output and self._drain_thread is not None:
            self._drain_thread.join(timeout=1)
        return self.stdout()

    def __enter__(self) -> "ServerProcess":
        return self.start()

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop()


def read_config_ports(config_path: Path) -> list[int]:
    """Naively parse listen directives to discover which ports to wait for."""
    ports = []
    with open(config_path, "r") as f:
        for line in f:
            line = line.strip()
            if line.startswith("listen"):
                parts = line.split()
                if len(parts) >= 2:
                    addr = parts[1].rstrip(";")
                    if ":" in addr:
                        try:
                            ports.append(int(addr.split(":")[-1]))
                        except ValueError:
                            pass
    return ports


def read_config_upload_stores(config_path: Path) -> list[Path]:
    """Naively parse `upload_store <dir>;` directives from a config file.

    The server expects these directories to already exist (it does not create
    them at startup), and the fixture wipes test_uploads/ before every run, so
    any configured upload directory that lives under test_uploads/ must be
    recreated here or POST uploads fail with 500 ("Unable to open file").
    """
    stores: list[Path] = []
    with open(config_path, "r") as f:
        for line in f:
            line = line.strip()
            if line.startswith("upload_store"):
                parts = line.split()
                if len(parts) >= 2:
                    p = Path(parts[1].rstrip(";"))
                    if not p.is_absolute():
                        p = PROJECT_ROOT / p
                    stores.append(p)
    return stores


@pytest.fixture
def server():
    """Function-scoped server running the default test configuration.

    A fresh server is started for each test to guarantee isolation and to
    recover from any test that crashes the server process.
    """
    with ServerProcess(DEFAULT_CONFIG) as srv:
        for port in read_config_ports(DEFAULT_CONFIG):
            if not wait_for_server("127.0.0.1", port, timeout=10):
                raise RuntimeError(f"Server did not become ready on port {port}")
        yield srv


@pytest.fixture
def server_with_config(tmp_path: Path):
    """Function-scoped server with a custom configuration."""

    class _Starter:
        def __init__(self):
            self._srv = None

        def start(self, config_text: str, *, expect_success: bool = True):
            config_file = tmp_path / "webserv_test.conf"
            config_file.write_text(config_text)
            self._srv = ServerProcess(config_file, expect_success=expect_success)
            self._srv.start()
            if expect_success:
                for port in read_config_ports(config_file):
                    if not wait_for_server("127.0.0.1", port, timeout=10):
                        self._srv.stop()
                        raise RuntimeError(f"Server did not become ready on port {port}")
            return self._srv

        def stop(self):
            if self._srv is not None:
                self._srv.stop()

    starter = _Starter()
    try:
        yield starter.start
    finally:
        starter.stop()
