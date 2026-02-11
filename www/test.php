<?php
// Show basic CGI variables
header("Content-Type: text/plain");  // plain text for easier reading
header("Status: 402 trans");

echo "Hello from PHP CGI!\n\n";

// Print some important CGI variables
echo "REQUEST_METHOD: " . ($_SERVER['REQUEST_METHOD'] ?? '') . "\n";
echo "SCRIPT_NAME: " . ($_SERVER['SCRIPT_NAME'] ?? '') . "\n";
echo "SCRIPT_FILENAME: " . ($_SERVER['SCRIPT_FILENAME'] ?? '') . "\n";
echo "QUERY_STRING: " . ($_SERVER['QUERY_STRING'] ?? '') . "\n";
echo "SERVER_NAME: " . ($_SERVER['SERVER_NAME'] ?? '') . "\n";
echo "SERVER_PORT: " . ($_SERVER['SERVER_PORT'] ?? '') . "\n";
echo "REMOTE_ADDR: " . ($_SERVER['REMOTE_ADDR'] ?? '') . "\n\n";

// Show GET parameters (from QUERY_STRING)
echo "GET parameters:\n";
print_r($_GET);

// Show all HTTP headers passed
echo "\nHTTP headers:\n";
foreach ($_SERVER as $key => $value) {
    if (strpos($key, 'HTTP_') === 0) {
        echo "$key: $value\n";
    }
}
 
