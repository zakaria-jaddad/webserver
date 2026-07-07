#!/usr/bin/env python3
import os
import sys
from email.parser import BytesParser
from email.policy import default

# 1. READ RAW BINARY CONTENT FROM STANDARD INPUT
content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
content_type = os.environ.get("CONTENT_TYPE", "")

# Safeguard against uninitialized reads
if content_length > 0:
    raw_body = sys.stdin.buffer.read(content_length)
else:
    raw_body = b""

# 2. MULTIPART FORM BOUNDARY PARSING MATRIX
# We synthesize a dummy email structure so the native email engine handles headers and binary streams easily
fake_email_headers = f"Content-Type: {content_type}\r\n\r\n".encode('utf-8')
full_stream = fake_email_headers + raw_body

msg_parsed = BytesParser(policy=default).parsebytes(full_stream)

# Initialize fallback values
fan = "Anonymous Fan"
song = "Unspecified Track"
msg_text = "(No urgent message left behind)"
file_status = "No file attached to payload."

# Loop through multipart payload chunks
if msg_parsed.is_multipart():
    for part in msg_parsed.iter_parts():
        name = part.get_param('name', header='content-disposition')
        
        if name == "fan_name":
            fan = part.get_content().strip()
        elif name == "song_title":
            song = part.get_content().strip()
        elif name == "message":
            val = part.get_content().strip()
            if val: msg_text = val
        elif name == "attachment":
            filename = part.get_filename()
            if filename:
                file_bytes = part.get_payload(decode=True)
                
                # Create a local upload repository inside STARRY server workspace
                upload_dir = "./www/cgi-bin/uploads"
                os.makedirs(upload_dir, exist_ok=True)
                
                # Clean up filename paths to prevent traversal vulnerabilities
                safe_filename = os.path.basename(filename)
                save_path = os.path.join(upload_dir, safe_filename)
                
                # Write file safely as an isolated binary stream
                with open(save_path, "wb") as f:
                    f.write(file_bytes)
                
                # file_status = f"Successfully saved file artifact as [ {safe_filename} ] inside STARRY archives!"
                file_status = (
                        f'Successfully saved '
                        f'<a href="/cgi-bin/uploads/{safe_filename}" target="_blank">'
                        f'{safe_filename}</a>'
                    )
else:
    # Fallback to standard application/x-www-form-urlencoded if file-upload was skipped
    import urllib.parse
    decoded_body = raw_body.decode("utf-8", errors="replace")
    data = urllib.parse.parse_qs(decoded_body)
    fan = data.get("fan_name", [fan])[0]
    song = data.get("song_title", [song])[0]
    msg_text = data.get("message", [msg_text])[0]

# 3. HTTP CGI RESPONSE GATEWAY PRINT ENGINE
print("Status: 200 OK")
print("Content-Type: text/html; charset=utf-8\r\n\r\n")

print(f"""<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <title>Accepted Request | Kessoku SERVER</title>
    <link
      rel="icon"
      type="image/jpeg"
      href="../sites/resources/bocchi_icon.jpg"
    />
    <style>
        :root {{
            --pink-bocchi: #ffb1cb;
            --blue-kita: #82e3f5;
            --yellow-nijika: #fff176;
            --purple-ryo: #b39ddb;
            --dark-punk: #121212;
            --white-paper: #f4f4f4;
            --neon-magenta: #ff007f;
            --tan-cardboard: #f7eed3;
        }}

        * {{ box-sizing: border-box; margin: 0; padding: 0; }}

        body {{
            background-color: var(--dark-punk);
            font-family: monospace;
            color: var(--white-paper);
            min-height: 100vh;
            padding: 4rem 1rem;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
        }}

        /* FIXED BACKGROUND VIEWPORT TEXTURE SCREEN */
        .hero-bg-overlay {{
            position: fixed;
            inset: 0;
            z-index: 1;
            pointer-events: none;
            opacity: 0.15;
        }}
        .halftone-pattern {{
            position: absolute;
            inset: 0;
            background-image: radial-gradient(var(--white-paper) 15%, transparent 16%);
            background-size: 12px 12px;
        }}

        .receipt-wrapper {{
            position: relative;
            z-index: 10;
            width: 100%;
            max-width: 580px;
            background-color: var(--white-paper);
            border: 5px solid var(--dark-punk);
            padding: 3rem 2rem 2.5rem 2rem;
            color: var(--dark-punk);
            box-shadow: 16px 20px 0px rgba(0,0,0,0.95);
            transform: rotate(1deg);
        }}

        .receipt-top-tape {{
            position: absolute;
            top: -20px;
            left: 35%;
            transform: rotate(-4deg);
            width: 120px;
            height: 30px;
            background-color: rgba(230, 222, 190, 0.85);
            border-left: 2px dashed rgba(0,0,0,0.1);
            border-right: 2px dashed rgba(0,0,0,0.1);
        }}

        .receipt-header {{
            background-color: var(--dark-punk);
            color: var(--white-paper);
            text-align: center;
            font-weight: 900;
            font-size: 1.4rem;
            padding: 0.5rem;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 2rem;
        }}

        .log-row {{
            border-bottom: 2px dashed #ccc;
            padding: 0.8rem 0;
            font-size: 1.1rem;
        }}
        .log-row strong {{
            text-transform: uppercase;
            color: var(--neon-magenta);
        }}

        .msg-box {{
            background-color: var(--tan-cardboard);
            border: 2px solid var(--dark-punk);
            padding: 1rem;
            margin-top: 0.5rem;
            font-style: italic;
        }}

        .status-alert-strip {{
            background-color: var(--blue-kita);
            font-weight: 900;
            padding: 0.5rem;
            margin-top: 1.5rem;
            border: 2px solid var(--dark-punk);
            font-size: 0.9rem;
            text-transform: uppercase;
        }}

        .return-btn {{
            display: block;
            text-align: center;
            margin-top: 2.5rem;
            font-weight: 900;
            text-transform: uppercase;
            text-decoration: none;
            background-color: var(--dark-punk);
            color: var(--yellow-nijika);
            padding: 0.8rem;
            border: 3px solid var(--dark-punk);
            box-shadow: 4px 4px 0px var(--neon-magenta);
            transition: transform 0.1s ease, box-shadow 0.1s ease;
        }}
        .return-btn:hover {{
            transform: translate(-2px, -2px);
            box-shadow: 6px 6px 0px var(--dark-punk);
            background-color: var(--neon-magenta);
            color: var(--white-paper);
        }}
    </style>
</head>
<body>

    <div class="hero-bg-overlay" aria-hidden="true">
        <div class="halftone-pattern"></div>
    </div>

    <main class="receipt-wrapper">
        <div class="receipt-top-tape"></div>
        
        <h1 class="receipt-header">🔥 REQUEST ACCEPTED AT STARRY</h1>
        
        <div class="log-row">
            <strong>Fan Identity:</strong> {fan}
        </div>
        
        <div class="log-row">
            <strong>Track Target:</strong> {song}
        </div>
        
        <div class="log-row">
            <strong>Anxious Dedication:</strong>
            <div class="msg-box">{msg_text}</div>
        </div>

        <div class="status-alert-strip">
            💾 PIPELINE METRIC: {file_status}
        </div>

        <a href="../sites/song_request.html" class="return-btn">← SEND ANOTHER DEMO REQUEST</a>
    </main>

</body>
</html>""")
