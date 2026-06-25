#!/usr/bin/env python3

import os
import sys

# CGI header
print("Content-Type: text/html")
print()

# Read POST body if present
content_length = os.environ.get("CONTENT_LENGTH", "0")

body = ""
if content_length.isdigit() and int(content_length) > 0:
    body = sys.stdin.read(int(content_length))

# HTML response
print("""
<!DOCTYPE html>
<html>
<head>
    <title>CGI Test</title>
</head>
<body>
    <h1>CGI is working!</h1>

    <h2>Environment Variables</h2>
    <table border="1">
        <tr>
            <th>Variable</th>
            <th>Value</th>
        </tr>
""")

for key in sorted(os.environ.keys()):
    print(f"""
        <tr>
            <td>{key}</td>
            <td>{os.environ[key]}</td>
        </tr>
    """)

print("""
    </table>
""")

if body:
    print(f"""
    <h2>POST Body</h2>
    <pre>{body}</pre>
    """)

print("""
</body>
</html>
""")
