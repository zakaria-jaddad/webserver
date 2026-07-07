<?php
/**
 * Minimal CGI test script for the webserv test suite.
 * Echoes request method, query string, content length and request body.
 */
echo "Content-type: text/plain\r\n\r\n";
echo "REQUEST_METHOD=" . getenv("REQUEST_METHOD") . "\n";
echo "QUERY_STRING=" . getenv("QUERY_STRING") . "\n";
echo "CONTENT_LENGTH=" . getenv("CONTENT_LENGTH") . "\n";
echo "CONTENT_TYPE=" . getenv("CONTENT_TYPE") . "\n";
echo "SCRIPT_NAME=" . getenv("SCRIPT_NAME") . "\n";
echo "PATH_INFO=" . getenv("PATH_INFO") . "\n";
echo "SERVER_NAME=" . getenv("SERVER_NAME") . "\n";
echo "SERVER_PORT=" . getenv("SERVER_PORT") . "\n";
echo "REMOTE_ADDR=" . getenv("REMOTE_ADDR") . "\n";

$body = file_get_contents("php://input");
if ($body !== false && strlen($body) > 0) {
    echo "BODY=" . $body . "\n";
}
?>
