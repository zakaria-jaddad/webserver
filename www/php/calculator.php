<?php
header("Content-Type: text/html");

$a  = isset($_REQUEST['a'])  ? (float)$_REQUEST['a']  : 0;
$b  = isset($_REQUEST['b'])  ? (float)$_REQUEST['b']  : 0;
$op = isset($_REQUEST['op']) ? $_REQUEST['op']         : 'add';

$result = 0;
$error  = "";

switch ($op) {
    case 'add': $result = $a + $b; break;
    case 'sub': $result = $a - $b; break;
    case 'mul': $result = $a * $b; break;
    case 'div':
        if ($b != 0) $result = $a / $b;
        else $error = "Division by zero!";
        break;
    default: $error = "Unknown operator";
}

$ops = ['add'=>'+','sub'=>'−','mul'=>'×','div'=>'÷'];
$op_sym = isset($ops[$op]) ? $ops[$op] : '?';
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>CGI Calculator</title>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=Fira+Code:wght@400;500&display=swap" rel="stylesheet">
  <style>
    *,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
    body{
      background:#080c14;color:#e2e8f0;font-family:'Inter',system-ui,sans-serif;
      min-height:100vh;display:flex;align-items:center;justify-content:center;padding:24px;
      background-image:radial-gradient(ellipse 60% 40% at 50% 0%,rgba(59,130,246,0.1) 0%,transparent 60%);
    }
    .wrapper{width:100%;max-width:420px;}
    .header{display:flex;align-items:center;gap:10px;margin-bottom:24px;}
    .header .icon{font-size:24px;}
    .header h1{font-size:18px;font-weight:600;}
    .header .badge{
      margin-left:auto;font-family:'Fira Code',monospace;font-size:10px;
      padding:3px 9px;border-radius:20px;color:#3b82f6;
      background:rgba(59,130,246,0.1);border:1px solid rgba(59,130,246,0.25);
    }
    .result-box{
      background:#0e1420;border:1px solid #1d2636;border-radius:10px;
      padding:20px 24px;margin-bottom:20px;text-align:right;
      position:relative;overflow:hidden;
    }
    .result-box::before{
      content:'';position:absolute;top:0;left:0;right:0;height:1px;
      background:linear-gradient(90deg,transparent,rgba(59,130,246,0.4),transparent);
    }
    .result-expr{font-size:13px;color:#64748b;font-family:'Fira Code',monospace;margin-bottom:4px;}
    .result-value{font-size:40px;font-weight:700;font-family:'Fira Code',monospace;color:#22c55e;}
    .result-error{font-size:18px;font-weight:600;color:#ef4444;}
    .calc-form{background:#111827;border:1px solid #1d2636;border-radius:10px;padding:20px;margin-bottom:16px;}
    .form-row{display:grid;grid-template-columns:1fr auto 1fr;gap:8px;align-items:end;margin-bottom:12px;}
    .field label{display:block;font-size:11px;color:#64748b;margin-bottom:4px;font-family:'Fira Code',monospace;}
    .field input{
      width:100%;padding:9px 12px;background:#080c14;border:1px solid #1d2636;
      border-radius:6px;color:#e2e8f0;font-family:'Fira Code',monospace;font-size:15px;
      outline:none;transition:border-color 0.15s;
    }
    .field input:focus{border-color:#3b82f6;}
    .op-sym{font-size:22px;font-weight:700;color:#3b82f6;text-align:center;padding-bottom:4px;font-family:'Fira Code',monospace;}
    .op-row{display:grid;grid-template-columns:repeat(4,1fr);gap:6px;margin-bottom:16px;}
    .op-btn{
      padding:9px;background:#080c14;border:1px solid #1d2636;border-radius:6px;
      color:#94a3b8;font-family:'Fira Code',monospace;font-size:16px;cursor:pointer;
      transition:all 0.15s;text-align:center;
    }
    .op-btn:hover{border-color:#3b82f6;color:#3b82f6;}
    .op-btn.selected{border-color:#3b82f6;background:rgba(59,130,246,0.12);color:#3b82f6;}
    .submit-btn{
      width:100%;padding:11px;background:#3b82f6;color:#fff;border:none;
      border-radius:8px;font-size:14px;font-weight:600;cursor:pointer;
      font-family:'Inter',sans-serif;transition:background 0.15s;
    }
    .submit-btn:hover{background:#2563eb;}
    .info-row{
      background:#0e1420;border:1px solid #1d2636;border-radius:8px;padding:12px 14px;
      font-size:12px;color:#475569;font-family:'Fira Code',monospace;margin-top:16px;
    }
    .info-row span{color:#38bdf8;}
    .back-link{
      display:block;text-align:center;margin-top:20px;
      color:#475569;font-size:12px;text-decoration:none;font-family:'Fira Code',monospace;
    }
    .back-link:hover{color:#3b82f6;}
  </style>
</head>
<body>
  <div class="wrapper">
    <div class="header">
      <span class="icon">🧮</span>
      <h1>CGI Calculator</h1>
      <span class="badge">PHP CGI</span>
    </div>

    <div class="result-box">
      <?php if ($error): ?>
        <div class="result-expr">Error</div>
        <div class="result-error"><?= htmlspecialchars($error) ?></div>
      <?php elseif (isset($_REQUEST['a'])): ?>
        <div class="result-expr"><?= $a . ' ' . $op_sym . ' ' . $b ?></div>
        <div class="result-value"><?= is_float($result) ? rtrim(rtrim(number_format($result, 10), '0'), '.') : $result ?></div>
      <?php else: ?>
        <div class="result-expr">result</div>
        <div class="result-value">—</div>
      <?php endif; ?>
    </div>

    <div class="calc-form">
      <form method="get" id="calcform">
        <div class="form-row">
          <div class="field"><label>a</label><input type="number" name="a" step="any" value="<?= htmlspecialchars($_REQUEST['a'] ?? '') ?>" required></div>
          <div class="op-sym" id="sym_disp"><?= htmlspecialchars($op_sym) ?></div>
          <div class="field"><label>b</label><input type="number" name="b" step="any" value="<?= htmlspecialchars($_REQUEST['b'] ?? '') ?>" required></div>
        </div>
        <div class="op-row">
          <?php foreach(['add'=>'+','sub'=>'−','mul'=>'×','div'=>'÷'] as $k=>$sym): ?>
          <button type="button" class="op-btn <?= $op===$k?'selected':'' ?>"
                  onclick="setOp('<?=$k?>','<?=$sym?>')"><?= $sym ?></button>
          <?php endforeach; ?>
        </div>
        <input type="hidden" name="op" id="op_input" value="<?= htmlspecialchars($op) ?>">
        <button type="submit" class="submit-btn">Calculate</button>
      </form>
    </div>

    <div class="info-row">
      REQUEST_METHOD: <span><?= $_SERVER['REQUEST_METHOD'] ?></span> ·
      QUERY_STRING: <span><?= htmlspecialchars($_SERVER['QUERY_STRING']) ?></span>
    </div>

    <a class="back-link" href="/">← Back to dashboard</a>
  </div>
  <script>
    function setOp(k, sym) {
      document.getElementById('op_input').value = k;
      document.getElementById('sym_disp').textContent = sym;
      document.querySelectorAll('.op-btn').forEach(b => b.classList.remove('selected'));
      event.target.classList.add('selected');
    }
  </script>
</body>
</html>
