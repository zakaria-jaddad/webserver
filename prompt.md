You are an autonomous AI software engineer working inside a Docker container with full access to the project source code.

## Environment

* You may install any tools, libraries, or dependencies you need to complete the task (Python packages, testing frameworks, networking tools, etc.).
* You have read/write access to the project.
* Work autonomously and make reasonable engineering decisions.

## Project Context

This project is my implementation of the 42 School Webserver (`webserv`) project.

The file `subject.md` is the authoritative specification. Treat it as the source of truth for all expected behavior. Every test should be derived from requirements defined in `subject.md`.

## Objective

Create a comprehensive automated test suite for the project using Python.

## Requirements

### 1. Test Organization

* Place all tests inside the `test/` directory.
* Organize tests logically by feature or requirement.
* Use descriptive filenames and test names.
* The test suite should be easy to extend and maintain.

### 2. Coverage

Generate tests covering every requirement described in `subject.md`, including but not limited to:

* HTTP request parsing

  * Request line
  * Methods
  * URI parsing
  * HTTP version
  * Headers
  * Invalid requests
  * Malformed requests
  * Header limits
  * Chunked requests (if required)
  * Body parsing

* Server behavior

  * Startup
  * Shutdown
  * Multiple ports
  * Multiple virtual servers
  * Configuration parsing
  * Error handling

* HTTP responses

  * Correct status codes
  * Required headers
  * Content-Length
  * Connection handling
  * Error pages
  * Redirects

* Static file serving

  * Existing files
  * Missing files
  * Directory handling
  * Index files
  * Autoindex (if required)
  * MIME types

* Supported HTTP methods

  * GET
  * POST
  * DELETE
  * Any additional methods required by the subject

* CGI execution (if required)

  * Environment variables
  * Request body handling
  * Response parsing
  * Error cases
  * Timeouts

* Upload handling (if required)

* Client connection management

  * Persistent connections
  * Connection close
  * Large requests
  * Multiple simultaneous clients

* Compliance with every requirement described in `subject.md`.

### 3. Test Quality

Tests should:

* Be deterministic.
* Be independent from one another.
* Clearly explain what requirement they verify.
* Fail with useful error messages.
* Include both positive and negative test cases.
* Cover edge cases and malformed input.
* Stress-test the server where appropriate.

### 4. Bug Reporting

Whenever you discover behavior that does not match the specification in `subject.md`:

* Create or update a file named `BUG.txt` in the project root.
* Document each issue with:

  * A unique bug number.
  * The violated requirement.
  * Steps to reproduce.
  * Expected behavior.
  * Actual behavior.
  * Severity (Critical, High, Medium, Low).
  * Which test exposes the bug.

Append new bugs without removing existing entries.

### 5. Test Execution

Provide a simple way to execute the entire suite (for example using `pytest`).

If helper scripts or fixtures are needed, create them.

The tests should automatically:

* Build the server if necessary.
* Launch the server.
* Wait until it is ready.
* Execute the tests.
* Shut the server down cleanly.

### 6. Final Goal

The end result should be a production-quality regression test suite that validates the entire project against `subject.md` and makes it easy to detect future regressions.

Do not stop after generating a few tests. Continue until every requirement in `subject.md` has corresponding automated tests. Report every discrepancy you find in `BUG.txt`.

