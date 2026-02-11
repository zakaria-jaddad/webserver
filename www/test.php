#!/usr/bin/env php
<?php

// Required header for CGI
echo "Content-Type: text/plain\r\n\r\n";

// Simple output
echo "CGI test successful!\n";
echo "PHP version: " . phpversion() . "\n";
echo "Current time: " . date('Y-m-d H:i:s') . "\n";

?>
