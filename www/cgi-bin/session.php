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
<title>Anime Session Page</title>

<style>
body {
    background: linear-gradient(135deg, #ff9a9e, #fad0c4);
    font-family: "Comic Sans MS", cursive, sans-serif;
    text-align: center;
    padding: 40px;
    color: #4b0082;
}

h1 {
    font-size: 3em;
    text-shadow: 2px 2px #ffb6c1;
}

.card {
    background: rgba(255,255,255,0.7);
    border-radius: 20px;
    padding: 20px;
    margin: 20px auto;
    max-width: 700px;
    box-shadow: 0 10px 20px rgba(0,0,0,0.2);
}

.gif-container img {
    border: 6px solid #fff;
    border-radius: 20px;
    max-width: 80%;
}

input, button {
    padding: 10px;
    margin: 5px;
    border-radius: 10px;
    border: none;
}

button {
    background: #4b0082;
    color: white;
    cursor: pointer;
}
</style>
</head>

<body>

<h1>Welcome to Anime Session World 🌸</h1>

<div class="card">
    <h2>Status</h2>
    <p><?= $is_new ? 'New session created ✨' : 'Existing session found 🔄' ?></p>
    <p><b>Session ID:</b> <?= $session_id ?></p>
    <p><b>User:</b> <?= $name ?></p>
</div>

<div class="card gif-container">
    <img src="http://media.giphy.com/media/a6pzK009rlCak/giphy.gif" alt="Anime GIF">
</div>

<div class="card">
    <h2>Login Form</h2>
    <form action="/upload" method="post" enctype="multipart/form-data">
        <input type="text" name="username" placeholder="Username" required><br>
        <input type="password" name="password" placeholder="Password" required><br>

        <label><input type="checkbox" name="vehicle1"> Bike</label><br>
        <label><input type="checkbox" name="vehicle2"> Car</label><br>
        <label><input type="checkbox" name="vehicle3"> Boat</label><br>

        <input type="file" name="profile" required><br>
        <button type="submit">Login</button>
    </form>
</div>

<div class="card">
    <h2>Session Log</h2>
    <pre style="text-align:left; white-space:pre-wrap;"><?= htmlspecialchars($log) ?></pre>
</div>

</body>
</html>