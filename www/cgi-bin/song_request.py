#!/usr/bin/env python3
import os
import urllib.parse

content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
raw_body = os.read(0, content_length).decode("utf-8", errors="replace")
data = urllib.parse.parse_qs(raw_body)

fan = data.get("fan_name", ["anonymous"])[0]
song = data.get("song_title", ["(none)"])[0]
msg = data.get("message", [""])[0]

print("Status: 200 OK")
print("Content-Type: text/html; charset=utf-8\r\n\r\n")

print(f"""<!doctype html>
<html>
<head><meta charset="utf-8"><title>Request Received</title></head>
<body>
  <h1>🔥 Request Accepted at STARRY</h1>
  <p><b>Fan:</b> {fan}</p>
  <p><b>Song:</b> {song}</p>
  <p><b>Message:</b> {msg}</p>
  <a href="/request.html">Send another request</a>
</body>
</html>""")
