- server fixture (function-scoped)
    - Runs before each test that requests it.
    - Builds the binary with make all if sources changed.
    - Starts ./ft_webserver conf/test.conf as a subprocess.
    - Waits until all configured ports are listening.
    - Yields control to the test.
    - After the test finishes, it terminates/kills the server process.

- server_with_config fixture
    - Same lifecycle, but writes a temporary config file and starts the server with that config instead.
