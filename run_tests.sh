#!/usr/bin/env bash
# Run the full webserv automated test suite.
# Builds the server if necessary, starts it for each test, and reports results.

set -e

cd "$(dirname "$0")"

if [ ! -d "venv" ]; then
    echo "Creating Python virtual environment..."
    python3 -m venv venv
fi

if ! venv/bin/python -c "import pytest" 2>/dev/null; then
    echo "Installing pytest and requests..."
    venv/bin/pip install --upgrade pip
    venv/bin/pip install pytest requests
fi

echo "Running webserv test suite..."
venv/bin/python -m pytest test/ "$@"
