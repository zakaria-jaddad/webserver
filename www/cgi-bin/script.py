#!/usr/bin/env python3

import os
import sys
import html

# CGI header
print("Content-Type: text/html\r\n\r\n")
#print()

# Read POST body if present
try:
    length = int(os.environ.get("CONTENT_LENGTH", 0))
except ValueError:
    length = 0

body = sys.stdin.read(length) if length > 0 else ""
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
            <td>{html.escape(key)}</td>
            <td>{html.escape(os.environ[key])}</td>
        </tr>
    """)

print("""
    </table>
""")

if body:
    print(f"""
    <h2>POST Body</h2>
    <pre>{html.escape(body)}</pre>
    """)

print("""
</body>
</html>
""")
