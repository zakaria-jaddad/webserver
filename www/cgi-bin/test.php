// <?php
// // session_counter.php
// // Simple session counter using HTTP cookies via CGI
//
// // Get cookie from environment variable
// $cookieHeader = getenv('HTTP_COOKIE') ?: '';
//
// // Parse cookies
// $cookies = array();
// if (!empty($cookieHeader)) {
//     $pairs = explode(';', $cookieHeader);
//     foreach ($pairs as $pair) {
//         $parts = explode('=', trim($pair), 2);
//         if (count($parts) == 2) {
//             $cookies[$parts[0]] = $parts[1];
//         }
//     }
// }
//
// // Check for existing session
// $sessionId = isset($cookies['PHPSESSID']) ? $cookies['PHPSESSID'] : '';
// $visitCount = 0;
//
// if (!empty($sessionId)) {
//     // Try to read existing session file
//     $sessionFile = '/tmp/webserv_sessions/' . basename($sessionId) . '.txt';
//     if (file_exists($sessionFile)) {
//         $data = unserialize(file_get_contents($sessionFile));
//         $visitCount = isset($data['count']) ? (int)$data['count'] : 0;
//     } else {
//         // Session ID provided but file missing — create new
//         $sessionId = '';
//     }
// }
//
// // New session
// if (empty($sessionId)) {
//     $sessionId = bin2hex(random_bytes(16));
//     $visitCount = 0;
// }
//
// // Increment count
// $visitCount++;
//
// // Save session
// $sessionDir = '/tmp/webserv_sessions/';
// if (!is_dir($sessionDir)) {
//     mkdir($sessionDir, 0777, true);
// }
// $sessionFile = $sessionDir . basename($sessionId) . '.txt';
// file_put_contents($sessionFile, serialize(array('count' => $visitCount)));
//
// // Output HTTP response
// header('Content-Type: text/html');
// header('Set-Cookie: PHPSESSID=' . $sessionId . '; Path=/; Max-Age=3600');
//
// echo "<!DOCTYPE html>\n";
// echo "<html>\n";
// echo "<head><title>Session Counter</title></head>\n";
// echo "<body>\n";
// echo "<h1>Visit Count: " . $visitCount . "</h1>\n";
// echo "<p>Session ID: " . htmlspecialchars($sessionId) . "</p>\n";
// echo "<p>Refresh to increment.</p>\n";
// echo "</body>\n";
// echo "</html>\n";
// ?>

<?php
$cookie_header = isset($_SERVER['HTTP_COOKIE']) ? $_SERVER['HTTP_COOKIE'] : '';
$session_id    = '';

if (preg_match('/session_id=([a-f0-9]+)/', $cookie_header, $m))
    $session_id = $m[1];

$is_new = empty($session_id);

if ($is_new) {
    $session_id = bin2hex(random_bytes(8));
    header("Set-Cookie: session_id=$session_id; Path=/; HttpOnly");
}

$name = isset($_GET['name']) ? htmlspecialchars($_GET['name']) : 'anonymous';

$log_file = 'session_log.txt';
$entry    = date('H:i:s') . " session=$session_id name=$name\n";
file_put_contents($log_file, $entry, FILE_APPEND);

$log = file_exists($log_file) ? file_get_contents($log_file) : 'empty';

echo "Content-Type: text/html\r\n\r\n";
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Session Demo — webserv</title>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=Fira+Code:wght@400;500&display=swap" rel="stylesheet">
  <style>
    *,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
    body{
      background:#080c14;color:#e2e8f0;font-family:'Inter',system-ui,sans-serif;
      min-height:100vh;padding:32px;
      background-image:radial-gradient(ellipse 60% 40% at 50% 0%,rgba(168,85,247,0.08) 0%,transparent 60%);
    }
    .wrapper{max-width:700px;margin:0 auto;}
    .page-header{display:flex;align-items:center;gap:12px;margin-bottom:28px;}
    .page-header h1{font-size:22px;font-weight:700;}
    .page-header .badge{
      font-family:'Fira Code',monospace;font-size:11px;padding:3px 10px;border-radius:20px;
      color:#a855f7;background:rgba(168,85,247,0.1);border:1px solid rgba(168,85,247,0.3);
    }
    .status-banner{
      display:flex;align-items:center;gap:12px;padding:14px 18px;
      border-radius:10px;margin-bottom:20px;border:1px solid;
    }
    .status-banner.new   {background:rgba(168,85,247,0.08);border-color:rgba(168,85,247,0.3);}
    .status-banner.exist {background:rgba(34,197,94,0.08); border-color:rgba(34,197,94,0.3);}
    .status-icon{font-size:22px;}
    .status-text strong{display:block;font-size:15px;font-weight:600;}
    .status-text span{font-size:12px;color:#64748b;}
    .status-banner.new   .status-text strong{color:#a855f7;}
    .status-banner.exist .status-text strong{color:#22c55e;}
    .grid{display:grid;grid-template-columns:1fr 1fr;gap:14px;margin-bottom:20px;}
    @media(max-width:600px){.grid{grid-template-columns:1fr;}}
    .info-card{background:#111827;border:1px solid #1d2636;border-radius:10px;padding:16px;}
    .info-card h3{font-size:11px;font-weight:600;text-transform:uppercase;letter-spacing:.08em;color:#64748b;margin-bottom:10px;padding-bottom:8px;border-bottom:1px solid #1d2636;}
    .kv-row{display:flex;flex-direction:column;gap:2px;margin-bottom:8px;}
    .kv-row:last-child{margin-bottom:0;}
    .kv-label{font-size:10px;color:#475569;font-family:'Fira Code',monospace;}
    .kv-value{font-size:12px;color:#38bdf8;font-family:'Fira Code',monospace;word-break:break-all;}
    .log-card{background:#111827;border:1px solid #1d2636;border-radius:10px;padding:16px;margin-bottom:20px;}
    .log-card h3{font-size:11px;font-weight:600;text-transform:uppercase;letter-spacing:.08em;color:#64748b;margin-bottom:10px;padding-bottom:8px;border-bottom:1px solid #1d2636;}
    .log-pre{background:#080c14;border:1px solid #1d2636;border-radius:6px;padding:12px;font-family:'Fira Code',monospace;font-size:12px;color:#22c55e;white-space:pre-wrap;word-break:break-all;max-height:180px;overflow-y:auto;line-height:1.7;}
    .link-row-group{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:20px;}
    .test-link{
      padding:8px 16px;border-radius:6px;font-size:13px;text-decoration:none;
      background:#0e1420;border:1px solid #1d2636;color:#94a3b8;transition:all 0.15s;
    }
    .test-link:hover{border-color:#a855f7;color:#a855f7;}
    .back-link{display:block;margin-top:8px;font-size:12px;color:#475569;text-decoration:none;font-family:'Fira Code',monospace;}
    .back-link:hover{color:#3b82f6;}
  </style>
</head>
<body>
  <div class="wrapper">
    <div class="page-header">
      <span style="font-size:24px">🍪</span>
      <h1>Session &amp; Cookies Demo</h1>
      <span class="badge">PHP CGI</span>
    </div>

    <div class="status-banner <?= $is_new ? 'new' : 'exist' ?>">
      <span class="status-icon"><?= $is_new ? '✨' : '🔄' ?></span>
      <div class="status-text">
        <strong><?= $is_new ? 'NEW session created' : 'EXISTING session found' ?></strong>
        <span>Session ID: <?= $session_id ?></span>
      </div>
    </div>

    <div class="grid">
      <div class="info-card">
        <h3>🍪 Cookie</h3>
        <div class="kv-row">
          <span class="kv-label">Status</span>
          <span class="kv-value"><?= $is_new ? 'Set-Cookie sent' : 'Cookie received' ?></span>
        </div>
        <div class="kv-row">
          <span class="kv-label">session_id</span>
          <span class="kv-value"><?= $session_id ?></span>
        </div>
        <div class="kv-row">
          <span class="kv-label">HTTP_COOKIE raw</span>
          <span class="kv-value"><?= $cookie_header ?: '(none)' ?></span>
        </div>
      </div>

      <div class="info-card">
        <h3>🌐 Request Env</h3>
        <div class="kv-row">
          <span class="kv-label">REQUEST_METHOD</span>
          <span class="kv-value"><?= $_SERVER['REQUEST_METHOD'] ?></span>
        </div>
        <div class="kv-row">
          <span class="kv-label">QUERY_STRING</span>
          <span class="kv-value"><?= htmlspecialchars($_SERVER['QUERY_STRING']) ?></span>
        </div>
        <div class="kv-row">
          <span class="kv-label">name param</span>
          <span class="kv-value"><?= $name ?></span>
        </div>
        <div class="kv-row">
          <span class="kv-label">__DIR__ (chdir proof)</span>
          <span class="kv-value"><?= __DIR__ ?></span>
        </div>
      </div>
    </div>

    <div class="log-card">
      <h3>📋 Session Log — persisted across requests</h3>
      <div class="log-pre"><?= htmlspecialchars($log) ?></div>
    </div>

    <div class="link-row-group">
      <a class="test-link" href="/cgi-bin/session.php?name=youssef">Visit as youssef</a>
      <a class="test-link" href="/cgi-bin/session.php?name=mehdi">Visit as mehdi</a>
      <a class="test-link" href="/cgi-bin/session.php?name=ayman">Visit as ayman</a>
    </div>

    <a class="back-link" href="/">← Back to dashboard</a>
  </div>
</body>
</html>
