#!/usr/bin/env python3
import os
import sys
import urllib.parse
import random

# Ensure standard output uses UTF-8 encoding for emojis
sys.stdout.reconfigure(encoding='utf-8')

# --- SESSION VAULT CONFIGURATION ---
# Path relative to where the CGI server spawns/executes the script
VAULT_DIR = "./www/cgi-bin/session_vault"

try:
    if not os.path.exists(VAULT_DIR):
        os.makedirs(VAULT_DIR, exist_ok=True)
except Exception:
    pass # Fallback safety if server environment restricts directory creation

def register_session(token):
    """Creates a physical file tracking this active session token."""
    if not token or "BAD_DATA" in token:
        return
    try:
        token_path = os.path.join(VAULT_DIR, token)
        with open(token_path, "w") as f:
            f.write("STATUS=ACTIVE\n")
    except Exception:
        pass

def revoke_session(token):
    """Deletes the session tracking file from disk."""
    if not token:
        return
    try:
        token_path = os.path.join(VAULT_DIR, token)
        if os.path.exists(token_path):
            os.remove(token_path)
    except Exception:
        pass

def is_session_valid(token):
    """Verifies if the token physically exists inside our hard-drive vault."""
    if not token or "BAD_DATA" in token:
        return False
    try:
        token_path = os.path.join(VAULT_DIR, token)
        return os.path.exists(token_path)
    except Exception:
        return False


# 1. PARSE INCOMING COOKIES & QUERY PARAMS
cookie_env = os.environ.get("HTTP_COOKIE", "")
query_env = os.environ.get("QUERY_STRING", "")

# Parse query string to handle button actions (?action=...)
params = urllib.parse.parse_qs(query_env)
action = params.get("action", [None])[0]

# Parse existing cookies
cookies = {}
if cookie_env:
    for c in cookie_env.split(";"):
        if "=" in c:
            k, v = c.strip().split("=", 1)
            cookies[k] = v.strip().rstrip(',').rstrip('\"')

COOKIE_NAME = "STARRY_PASS"
current_token = cookies.get(COOKIE_NAME, None)

# 2. PROCESS MUTATION ACTIONS & PREPARE SET-COOKIE HEADERS
headers = ["Content-Type: text/html; charset=utf-8"]

if action == "stamp":
    # If a previous token existed, clear it out first to avoid dead file accumulation
    if current_token:
        revoke_session(current_token)
        
    # Generate frantic pseudorandom hexadecimal string hash identifier
    pseudo_hash = f"STARRY_{hex(random.randint(0x10000000, 0xffffffff))[2:].upper()}_SYS"
    headers.append(f"Set-Cookie: {COOKIE_NAME}={pseudo_hash}; Path=/; Max-Age=300; SameSite=Strict")
    current_token = pseudo_hash
    
    # Commit token to the server hard-drive cache
    register_session(current_token)

elif action == "corrupt":
    # Revoke the actual clean token from the vault first
    if current_token:
        revoke_session(current_token)
        
    # Inject bad data token (will NOT be registered in the vault)
    corrupt_val = "BAD_DATA_ERR_MUTATION"
    headers.append(f"Set-Cookie: {COOKIE_NAME}={corrupt_val}; Path=/; Max-Age=300; SameSite=Strict")
    current_token = corrupt_val

elif action == "wipe":
    # Explicitly clear file footprint from the server vault
    if current_token:
        revoke_session(current_token)
    
    # Backdating structural limits flags eviction across standard clients
    headers.append(f"Set-Cookie: {COOKIE_NAME}=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT;")
    current_token = None

# 3. COMPUTE SERVER-SIDE UI STATE
unverified_display = "block"
verified_display = "none"
metadata_display = "none"
token_string = "NULL"
monitor_color = "#ff007f" # --neon-magenta
monitor_text = f"⚠️ Inbound Request Header:\nCookie: NULL\n\n[CGI Status: 401 Unauthorized - Access Denied]"

# Check state against the real server-side file system vault
if current_token:
    if "BAD_DATA" in current_token:
        monitor_text = f"👉 Inbound Request Header:\nCookie: {COOKIE_NAME}={current_token}\n\n[ALERT] map.find() FAILED: Token corrupted. Kicking client out..."
        monitor_color = "#ff3333"
    elif is_session_valid(current_token):
        unverified_display = "none"
        verified_display = "block"
        metadata_display = "block"
        token_string = current_token
        monitor_text = f"👉 Inbound Request Header:\nCookie: {COOKIE_NAME}={current_token}\n\n[CGI Status: Hard-Drive Vault File MATCH]"
        monitor_color = "#00ff00"
    else:
        # Token exists in browser but is missing from server vault (e.g. Server cleared files or token is forged)
        monitor_text = f"👉 Inbound Request Header:\nCookie: {COOKIE_NAME}={current_token}\n\n[ALERT] Vault Verification Failed: Token absent from cache."
        monitor_color = "#ffaa00"


# 4. PRINT HTTP CGI HEADERS (CRITICAL FOR WEBSERV)
for h in headers:
    print(h, end="\r\n")
print("", end="\r\n") # The mandatory empty line separating headers from the body

# 5. DUMP DYNAMIC HTML
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>STARRY Door Control // Session Logger</title>
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
      --white-paper: #f7eed3;
      --neon-magenta: #ff007f;
      --tan-cardboard: #f7eed3;
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

    .session-matrix {{
      display: grid;
      grid-template-columns: 1fr;
      gap: 2rem;
      max-width: 850px;
      width: 100%;
    }}

    @media (min-width: 768px) {{
      .session-matrix {{
        grid-template-columns: 1.2fr 1fr;
        align-items: start;
      }}
    }}

    .vip-pass-card {{
      background: var(--white-paper);
      color: var(--dark-punk);
      border: 5px solid var(--dark-punk);
      padding: 2rem;
      position: relative;
      box-shadow: 12px 12px 0px var(--purple-ryo);
      transform: rotate(-1.5deg);
    }}

    .vip-pass-card::before {{
      content: '';
      position: absolute;
      top: 15px;
      left: 50%;
      transform: translateX(-50%);
      width: 35px;
      height: 12px;
      background: var(--dark-punk);
      border-radius: 20px;
    }}

    .pass-header {{
      text-align: center;
      margin-top: 1rem;
      border-bottom: 3px dashed var(--dark-punk);
      padding-bottom: 1rem;
    }}

    .pass-title {{
      font-size: 1.8rem;
      font-weight: 900;
      text-transform: uppercase;
      letter-spacing: -1px;
      line-height: 1;
    }}

    .pass-title span {{
      background: var(--dark-punk);
      color: var(--pink-bocchi);
      padding: 0 6px;
      display: inline-block;
      margin-top: 4px;
    }}

    .pass-status-zone {{
      margin: 2rem 0;
      text-align: center;
      padding: 15px;
      border: 3px solid var(--dark-punk);
      background: #e8dfc5;
      position: relative;
    }}

    .ink-stamp {{
      font-size: 2.2rem;
      font-weight: 900;
      text-transform: uppercase;
      letter-spacing: 2px;
    }}

    .status-unverified {{
      color: #777;
      border: 4px dashed #777;
      padding: 10px;
      display: {unverified_display};
    }}

    .status-verified {{
      color: var(--neon-magenta);
      border: 5px double var(--neon-magenta);
      padding: 10px;
      display: {verified_display};
      background: rgba(255, 0, 127, 0.05);
      text-shadow: 2px 2px 0px rgba(0, 0, 0, 0.1);
      transform: scale(1) rotate(-8deg);
    }}

    .console-block {{
      background: #111;
      border: 4px solid var(--white-paper);
      padding: 1.5rem;
      box-shadow: 8px 8px 0px var(--dark-punk);
    }}

    .console-heading {{
      font-size: 0.9rem;
      font-weight: bold;
      text-transform: uppercase;
      background: var(--yellow-nijika);
      color: var(--dark-punk);
      padding: 4px 8px;
      margin-bottom: 1rem;
      display: inline-block;
    }}

    .raw-header-monitor {{
      background: var(--dark-punk);
      border: 2px solid #333;
      padding: 12px;
      font-size: 0.75rem;
      color: {monitor_color};
      overflow-x: auto;
      white-space: pre-wrap;
      word-break: break-all;
      height: 110px;
      margin-bottom: 1.5rem;
    }}

    .action-panel {{
      display: flex;
      flex-direction: column;
      gap: 0.75rem;
    }}

    .ctrl-btn {{
      width: 100%;
      padding: 12px;
      font-size: 0.9rem;
      font-weight: 900;
      text-transform: uppercase;
      cursor: pointer;
      border: 3px solid var(--dark-punk);
      box-shadow: 4px 4px 0px var(--dark-punk);
      text-decoration: none;
      text-align: center;
    }}

    .ctrl-btn:active {{
      transform: translate(4px, 4px);
      box-shadow: 0px 0px 0px var(--dark-punk);
    }}

    .btn-stamp   {{ background: var(--blue-kita); color: var(--dark-punk); }}
    .btn-corrupt {{ background: var(--yellow-nijika); color: var(--dark-punk); }}
    .btn-wipe    {{ background: #ff3333; color: white; }}

    .session-data-display {{
      font-size: 0.8rem;
      font-weight: bold;
      margin-top: 0.5rem;
      display: {metadata_display};
      word-break: break-all;
      border-top: 1px dashed var(--dark-punk);
      padding-top: 0.5rem;
    }}
    
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
  </style>
</head>
<body>
        <nav class="zine-nav" aria-label="Main Navigation">
            <a href="../index.html" class="nav-sticker tape-pink">🎸 Main Posting</a>
            <a href="../sites/song_request.html" class="nav-sticker tape-yellow">📝 Song Requests</a>
            <a href="backstage.py" class="nav-sticker tape-purple">🎛 Backstage</a>
            <a href="equipment.php" class="nav-sticker tape-blue">📦 Equipment</a>
            <a href="session.py" class="nav-sticker tape-white">🍪 Café</a>
            <a href="../sites/About_us.html" class="nav-sticker tape-dark">⚙ About us</a>
        </nav>

  <div class="session-matrix">
    
    <div class="vip-pass-card" id="vipPass">
      <div class="pass-header">
        <div class="pass-title">STARRY BACKSTAGE<br><span>PASS // ACCESS_02</span></div>
      </div>

      <div class="pass-status-zone">
        <div class="ink-stamp status-unverified">NO STAMP</div>
        <div class="ink-stamp status-verified">VIP ACCESS</div>
        <div class="session-data-display">
          TOKEN: <span style="color: var(--purple-ryo)">{token_string}</span>
        </div>
      </div>

      <div style="font-size: 0.7rem; text-align: center; font-weight: bold; color: #555;">
        [VALID FOR 300S // INK FADES VIA SERVER CONTROL]
      </div>
    </div>

    <div class="console-block">
      <div class="console-heading">HTTP Inbound Header Sniffer</div>
      <div class="raw-header-monitor">{monitor_text}</div>

      <div class="console-heading" style="background: var(--pink-bocchi);">Session Controls</div>
      
      <div class="action-panel">
        <a href="?action=stamp" class="ctrl-btn btn-stamp">Stamp Hand (Set-Cookie)</a>
        <a href="?action=corrupt" class="ctrl-btn btn-corrupt">Corrupt Cookie String</a>
        <a href="?action=wipe" class="ctrl-btn btn-wipe">Wipe Stamp (Delete Cookie)</a>
      </div>
    </div>

  </div>

</body>
</html>
""")
