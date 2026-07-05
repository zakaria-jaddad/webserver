#!/usr/bin/env python3
import os
import sys
import glob

# --- CONFIGURATION PATHS ---
# Relative to where your server binary executes, or absolute.
# Based on your target layout: pages are in 'www/pages/'
PAGES_DIR = os.path.join("www", "pages")

# 1. READ ALL CODES AND HTML FILES FROM TARGET DIR
error_files = []
if os.path.exists(PAGES_DIR):
    # Search for any html error layout templates (e.g., 404.html, 500.html)
    for file_path in glob.glob(os.path.join(PAGES_DIR, "*.html")):
        base_name = os.path.basename(file_path)
        # Extract potential status code number from file title if applicable
        code_guess = "".join(filter(str.isdigit, base_name)) or "???"
        size_kb = os.path.getsize(file_path) / 1024
        error_files.append({
            "name": base_name,
            "path": f"pages/{base_name}",
            "code": code_guess,
            "size": f"{size_kb:.1f} KB"
        })

# 2. OUTPUT RAW HTTP PROTOCOL COMPLIANT HEADERS FOR THE C++ CORE
print("Content-Type: text/html; charset=utf-8\r")
print("\r")

# 3. GENERATE FRONT-END COLLAGE PAYLOAD
html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Backstage Mixer | Kessoku SERVER</title>
  <style>
    /* --- DESIGN SYSTEM TOKENS --- */
    :root {{
      --pink-bocchi: #ffb1cb;
      --blue-kita: #82e3f5;
      --yellow-nijika: #fff176;
      --purple-ryo: #b39ddb;
      --dark-punk: #121212;
      --white-paper: #f7eed3;
      --neon-magenta: #ff007f;
    }}

    * {{
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: 'Courier New', Courier, monospace;
    }}

    body {{
      background-color: var(--dark-punk);
      background-image: radial-gradient(rgba(255,255,255,0.07) 15%, transparent 15%);
      background-size: 12px 12px;
      color: var(--white-paper);
      padding: 3rem 1rem;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
      overflow-x: hidden;
    }}

    /* --- FIXED ZINE NAVIGATION (STICKER TAPE) --- */
    .zine-nav {{
      position: fixed;
      top: 15%;
      right: 20px;
      display: flex;
      flex-direction: column;
      gap: 0.8rem;
      z-index: 100;
      transform: rotate(2deg);
    }}

    @media (max-width: 768px) {{
      .zine-nav {{
        position: static;
        transform: none;
        padding: 1rem;
        grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
        display: grid;
      }}
    }}

    .nav-sticker {{
      color: var(--dark-punk);
      font-weight: 900;
      text-decoration: none;
      padding: 0.6rem 1.2rem;
      font-size: 0.9rem;
      text-transform: uppercase;
      border: 2px solid var(--dark-punk);
      transition: transform 0.15s ease, box-shadow 0.15s ease;
      box-shadow: 4px 4px 0px var(--dark-punk);
    }}

    .tape-pink {{ background-color: var(--pink-bocchi); transform: rotate(-3deg); }}
    .tape-yellow {{ background-color: var(--yellow-nijika); transform: rotate(2deg); }}
    .tape-purple {{ background-color: var(--purple-ryo); transform: rotate(-1deg); }}
    .tape-blue {{ background-color: var(--blue-kita); transform: rotate(4deg); }}
    .tape-white {{ background-color: var(--white-paper); transform: rotate(-2deg); }}
    .tape-magenta {{ background-color: var(--neon-magenta); color: white; transform: rotate(1deg); }}
    .tape-dark {{ background-color: var(--dark-punk); color: var(--yellow-nijika); transform: rotate(-4deg); }}
    
    .nav-sticker:hover {{
      transform: scale(1.1) rotate(0deg);
      box-shadow: 6px 6px 0px var(--neon-magenta);
    }}

    /* --- CONTAINER --- */
    .storage-container {{
      background: #111111;
      border: 4px dashed var(--pink-bocchi);
      padding: 2rem;
      max-width: 750px;
      width: 100%;
      box-shadow: 10px 10px 0px var(--dark-punk);
      position: relative;
    }}

    .panel-header {{
      border-bottom: 4px double var(--white-paper);
      padding-bottom: 1rem;
      margin-bottom: 2rem;
    }}

    .system-tag {{
      background: var(--purple-ryo);
      color: #fff;
      display: inline-block;
      padding: 2px 8px;
      font-size: 0.75rem;
      font-weight: bold;
      text-transform: uppercase;
      margin-bottom: 0.5rem;
    }}

    h2 {{
      font-size: 2rem;
      color: var(--white-paper);
      text-transform: uppercase;
      letter-spacing: -1px;
    }}

    h2 span {{
      color: var(--pink-bocchi);
    }}

    .warning-banner {{
      background: var(--dark-punk);
      color: var(--yellow-nijika);
      border: 2px solid var(--yellow-nijika);
      padding: 10px;
      font-size: 0.8rem;
      margin-top: 0.5rem;
      font-style: italic;
    }}

    /* --- THE TAPE RACK GRID --- */
    .tape-rack {{
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
      gap: 1.5rem;
    }}

    /* --- INDIVIDUAL CASSETTE TAPE BLOCK --- */
    .tape-block {{
      background: var(--white-paper);
      color: var(--dark-punk);
      border: 3px solid var(--dark-punk);
      padding: 1rem;
      position: relative;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      min-height: 160px;
      box-shadow: 6px 6px 0px var(--blue-kita);
      transform: rotate(var(--random-rotation, -1deg));
      transition: transform 0.15s steps(2);
    }}

    .tape-block:nth-child(even) {{ --random-rotation: 1.5deg; box-shadow: 6px 6px 0px var(--pink-bocchi); }}
    .tape-block:nth-child(3n) {{ --random-rotation: -2deg; box-shadow: 6px 6px 0px var(--yellow-nijika); }}

    .tape-label {{
      background: var(--dark-punk);
      color: #fff;
      padding: 2px 4px;
      font-size: 0.7rem;
      font-weight: bold;
      text-transform: uppercase;
      margin-bottom: 0.5rem;
      display: inline-block;
    }}

    .file-name {{
      font-size: 1.1rem;
      font-weight: 900;
      word-break: break-all;
      line-height: 1.1;
      margin-bottom: 0.5rem;
    }}

    .file-meta {{
      font-size: 0.75rem;
      color: #555;
      font-weight: bold;
      margin-bottom: 1rem;
    }}

    /* --- DISPATCH TEST BUTTON --- */
    .test-btn {{
      background: var(--neon-magenta);
      color: white;
      border: 2px solid var(--dark-punk);
      padding: 6px;
      font-size: 0.8rem;
      font-weight: 900;
      text-transform: uppercase;
      cursor: pointer;
      width: 100%;
      text-align: center;
      box-shadow: 3px 3px 0px var(--dark-punk);
      transition: all 0.1s steps(1);
    }}

    .test-btn:hover {{
      background: var(--yellow-nijika);
      color: var(--dark-punk);
      box-shadow: 1px 1px 0px var(--dark-punk);
      transform: translate(2px, 2px);
    }}

    .empty-rack-notice {{
      grid-column: 1 / -1;
      text-align: center;
      padding: 2rem;
      border: 2px dashed rgba(255,255,255,0.2);
      color: #666;
      font-weight: bold;
    }}
  </style>
</head>
<body>

  <nav class="zine-nav" aria-label="Main Navigation">
    <a href="../index.html" class="nav-sticker tape-pink">🎸 Main Posting</a>
    <a href="../sites/song_request.html" class="nav-sticker tape-yellow">📝 Song Requests</a>
    <a href="backstage.py" class="nav-sticker tape-purple">🎛 Backstage</a>
    <a href="equipment.py" class="nav-sticker tape-blue">📦 Equipment</a>
    <a href="session.py" class="nav-sticker tape-white">🍪 Café</a>
    <a href="../sites/About_us.html" class="nav-sticker tape-dark">⚙ About us</a>
  </nav>

  <div class="storage-container">
    <div class="panel-header">
      <div class="system-tag">HTTP_ERR_ROUTING: ACTIVE</div>
      <h2>BACKSTAGE PANIC // <span>MIXER ROOM</span></h2>
      <div class="warning-banner">
        BOCH-OS WARNING: Click any audio track error template below to manually fire and preview its look through the client viewport.
      </div>
    </div>

    <div class="tape-rack" id="rackContainer">
"""

# Dynamic generation loop inside Python template matrix string
if not error_files:
    html_content += '<div class="empty-rack-notice">NO TEMPLATES FOUND IN www/pages/. BOCHI HAS ESCAPED ALL CRASH MESSAGES!</div>'
else:
    for file in error_files:
        html_content += f"""
      <div class="tape-block" data-uri="../{file['path']}">
        <div>
          <span class="tape-label">STATUS {file['code']}</span>
          <div class="file-name">{file['name']}</div>
          <div class="file-meta">SIZE: {file['size']}<br>DIR: /pages</div>
        </div>
        <button class="test-btn" onclick="previewError(this)">PREVIEW ERR</button>
      </div>
        """

html_content += """
    </div>
  </div>

  <script>
    // Asynchronous frontend testing interceptor
    function previewError(buttonComponent) {
      const tapeElement = buttonComponent.closest('.tape-block');
      const targetUri = tapeElement.getAttribute('data-uri');
      
      console.log("Navigating to preview structural error layout:", targetUri);
      // Directly transition the viewport frame over to show off the customized error template design
      window.location.href = targetUri;
    }
  </script>
</body>
</html>
"""

print(html_content)