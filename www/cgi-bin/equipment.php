<?php

$root = realpath(__DIR__ . "/uploads");   // document root

function listFiles($dir)
{
    $files = [];

    $iterator = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator(
            $dir,
            FilesystemIterator::SKIP_DOTS
        )
    );

    foreach ($iterator as $file)
    {
        if ($file->isFile())
            $files[] = $file;
    }

    return $files;
}

$files = listFiles($root);

?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Equipment Room | Kessoku SERVER</title>
    <link
      rel="icon"
      type="image/jpeg"
      href="../sites/resources/bocchi_icon.jpg"
    />
  <style>
    /* --- DESIGN SYSTEM TOKENS --- */
    :root {
            --pink-bocchi: #ffb1cb;
            --blue-kita: #82e3f5;
            --yellow-nijika: #fff176;
            --purple-ryo: #b39ddb;
            --dark-punk: #121212;
            --white-paper: #f4f4f4;
            --neon-magenta: #ff007f;
            --tan-cardboard: #f7eed3;
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: 'Courier New', Courier, monospace;
    }

    body {
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
    }

    /* ==========================================================================
            6. FIXed ZINE NAVIGATION (STICKER TAPE)
            ========================================================================== */
            .zine-nav {
                position: fixed;
                top: 15%;
                right: 20px;
                display: flex;
                flex-direction: column;
                gap: 0.8rem;
                z-index: 100;
                transform: rotate(2deg);
            }
        
            @media (max-width: 768px) {
                .zine-nav {
                position: static;
                transform: none;
                padding: 1rem;
                grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
                display: grid;
                }
            }
        
            .nav-sticker {
                color: var(--dark-punk);
                font-weight: 900;
                text-decoration: none;
                padding: 0.6rem 1.2rem;
                font-size: 0.9rem;
                text-transform: uppercase;
                border: 2px solid var(--dark-punk);
                transition: transform 0.15s ease, box-shadow 0.15s ease;
                box-shadow: 4px 4px 0px var(--dark-punk);
                }
            
            .tape-pink { background-color: var(--pink-bocchi); transform: rotate(-3deg); }
            .tape-yellow { background-color: var(--yellow-nijika); transform: rotate(2deg); }
            .tape-purple { background-color: var(--purple-ryo); transform: rotate(-1deg); }
            .tape-blue { background-color: var(--blue-kita); transform: rotate(4deg); }
            .tape-white { background-color: var(--white-paper); transform: rotate(-2deg); }
            .tape-magenta { background-color: var(--neon-magenta); color: white; transform: rotate(1deg); }
            .tape-dark { background-color: var(--dark-punk); color: var(--yellow-nijika); transform: rotate(-4deg); }
            
            .nav-sticker:hover {
                transform: scale(1.1) rotate(0deg);
                box-shadow: 6px 6px 0px var(--neon-magenta);
            }

    /* --- EQUIPMENT STORAGE CONTAINER --- */
    .storage-container {
      background: #111111;
      border: 4px dashed var(--yellow-nijika);
      padding: 2rem;
      max-width: 750px;
      width: 100%;
      box-shadow: 10px 10px 0px var(--dark-punk);
      position: relative;
    }

    .panel-header {
      border-bottom: 4px double var(--white-paper);
      padding-bottom: 1rem;
      margin-bottom: 2rem;
    }

    .system-tag {
      background: var(--neon-magenta);
      color: #fff;
      display: inline-block;
      padding: 2px 8px;
      font-size: 0.75rem;
      font-weight: bold;
      text-transform: uppercase;
      margin-bottom: 0.5rem;
    }

    h2 {
      font-size: 2rem;
      color: var(--white-paper);
      text-transform: uppercase;
      letter-spacing: -1px;
    }

    h2 span {
      color: var(--yellow-nijika);
    }

    .warning-banner {
      background: var(--dark-punk);
      color: var(--pink-bocchi);
      border: 2px solid var(--pink-bocchi);
      padding: 10px;
      font-size: 0.8rem;
      margin-top: 0.5rem;
      font-style: italic;
    }

    /* --- THE TAPE RACK GRID --- */
    .tape-rack {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
      gap: 1.5rem;
    }

    /* --- INDIVIDUAL CASSETTE TAPE BLOCK --- */
    .tape-block {
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
      transition: transform 0.2s steps(2), opacity 0.2s steps(2);
    }

    /* Dynamic tilt variations using standard CSS overrides */
    .tape-block:nth-child(even) { --random-rotation: 1.5deg; box-shadow: 6px 6px 0px var(--pink-bocchi); }
    .tape-block:nth-child(3n) { --random-rotation: -2deg; box-shadow: 6px 6px 0px var(--yellow-nijika); }

    .tape-label {
      background: var(--dark-punk);
      color: #fff;
      padding: 2px 4px;
      font-size: 0.7rem;
      font-weight: bold;
      text-transform: uppercase;
      margin-bottom: 0.5rem;
      display: inline-block;
    }

    .file-name {
      font-size: 1.1rem;
      font-weight: 900;
      word-break: break-all;
      line-height: 1.1;
      margin-bottom: 0.5rem;
    }

    .file-meta {
      font-size: 0.75rem;
      color: #555;
      font-weight: bold;
      margin-bottom: 1rem;
    }

    /* --- DESTRUCTION SWITCH (DELETE BUTTON) --- */
    .shred-btn {
      background: #ff3333;
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
    }

    .shred-btn:hover {
      background: var(--dark-punk);
      color: var(--neon-magenta);
      box-shadow: 1px 1px 0px var(--dark-punk);
      transform: translate(2px, 2px);
    }

    /* --- GLITCH OUT ANIMATION FOR DELETED TAPES --- */
    .tape-block.evicting {
      animation: tape-shred 0.3s steps(3) forwards;
      pointer-events: none;
    }

    @keyframes tape-shred {
      0% { transform: scale(1) skewX(0deg); opacity: 1; background: var(--neon-magenta); }
      50% { transform: scale(0.8) skewX(20deg); opacity: 0.5; background: var(--yellow-nijika); }
      100% { transform: scale(0) skewX(-20deg); opacity: 0; display: none; }
    }

    .empty-rack-notice {
      grid-column: 1 / -1;
      text-align: center;
      padding: 2rem;
      border: 2px dashed rgba(255,255,255,0.2);
      color: #666;
      font-weight: bold;
    }
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

<div class="storage-container">

<div class="panel-header">
<div class="system-tag">HTTP_METHOD: DELETE</div>
<h2>STAGE RACK // <span>EQUIPMENT ROOM</span></h2>
</div>

<div class="tape-rack" id="rackContainer">

<?php foreach ($files as $file):

    $absolute = $file->getPathname();

    $relative = "uploads" . str_replace($root, "", $absolute);

    $mime = mime_content_type($absolute);

    $size = filesize($absolute);

?>

<div class="tape-block" data-uri="<?= htmlspecialchars($relative) ?>">

<div>

<span class="tape-label"><?= htmlspecialchars($mime) ?></span>

<div class="file-name">
<?= htmlspecialchars(basename($absolute)) ?>
</div>

<div class="file-meta">
SIZE:
<?= number_format($size / 1024, 1) ?> KB
</div>

</div>

<button class="shred-btn" onclick="evictTape(this)">
SHRED TAPE
</button>

</div>

<?php endforeach; ?>

</div>

</div>

  <script>
    // Pure vanilla client-side execution loop
    function evictTape(buttonComponent) {
      const tapeElement = buttonComponent.closest('.tape-block');
      const targetUri = tapeElement.getAttribute('data-uri');

      // Add low-frame-rate structural breakdown sequence immediately to the UI
      tapeElement.classList.add('evicting');

      // Native zero-framework asynchronous client transmitter
      console.log("uri : ", targetUri);
      fetch(targetUri, {
        method: 'DELETE'
      })
      .then(response => {
        if (response.ok || response.status === 200 || response.status === 204) {
          // Keep it out of the layout flow after animation completes
          setTimeout(() => {
            tapeElement.remove();
            checkRackStatus();
          }, 300);
        } else {
          // Reset if our server threw a 500 or mismanaged poll loop back-out
          tapeElement.classList.remove('evicting');
          alert('SERVER DESTRUCTION ERROR: File refused to un-link from C++ backend.');
        }
      })
      .catch(error => {
        // Fallback for simulation/testing if server engine isn't bound to socket yet
        console.warn("Backend unreached. Simulating local hardware crunch sequence...");
        setTimeout(() => {
          tapeElement.remove();
          checkRackStatus();
        }, 300);
      });
    }

    function checkRackStatus() {
      const rack = document.getElementById('rackContainer');
      if (rack.children.length === 0) {
        rack.innerHTML = `<div class="empty-rack-notice">RACK VACANT. HARD DRIVE BREATHING EASILY. READY FOR HIGH-VOLTAGE INCOMING DATA blocks.</div>`;
      }
    }
  </script>

</body>
</html>
