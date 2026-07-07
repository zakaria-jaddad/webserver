#!/usr/bin/env bash
# Manual stress-test script for ft_webserver.
# Covers every mandatory requirement from the subject and evaluation sheets.
# Usage: ./ft_webserver conf/test.conf &  (then run this script)

set -u
HOST="127.0.0.1"
P1=2010
P2=2011
P3=2012

pass=0
fail=0

expect() {
    local code="$1" want="$2" desc="$3"
    if echo "$code" | grep -Eq "^(${want})$"; then
        echo "  [PASS] $desc -> $code"
        pass=$((pass+1))
    else
        echo "  [FAIL] $desc -> $code (expected $want)"
        fail=$((fail+1))
    fi
}

echo "===== ft_webserver manual stress test ====="

# ---- Configuration / listeners ----
echo "[CONFIG] multiple ports / virtual hosts"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P1/)" "200" "GET / on port $P1"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P2/)" "200" "GET / on port $P2"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P3/)" "200" "GET / on port $P3"

# ---- Redirection ----
echo "[REDIRECT] configured route"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P3/old)" "301|302|307|308" "GET /old redirects"

# ---- Error pages ----
echo "[ERRORS] status codes and default error pages"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P1/notfound)" "404" "404 for missing file"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" -X POST -d "" http://$HOST:$P2/)" "405" "405 for disallowed method"

# ---- Methods ----
echo "[METHODS] GET / POST / DELETE / UNKNOWN"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P1/index.html)" "200" "GET existing file"
printf 'posttest data' > www/posttest.txt
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" -X POST --data-binary @www/posttest.txt http://$HOST:$P1/)" "200|201|204" "POST allowed route"
echo "method-delete-test" > www/method-del.txt
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" -X DELETE http://$HOST:$P1/method-del.txt)" "200|204" "DELETE existing file"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" -X DELETE http://$HOST:$P1/method-del.txt)" "404" "DELETE missing file returns 404"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" -X PUT -d "" http://$HOST:$P1/)" "400|405|501" "UNKNOWN method rejected"

# ---- Static files / MIME / directory index / autoindex ----
echo "[STATIC] MIME, index, autoindex"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P1/sites/resources/yhossni.jpeg)" "200" "GET JPEG image"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{content_type}" http://$HOST:$P1/sites/resources/yhossni.jpeg)" "image/jpeg|image/jpg" "JPEG content-type"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P1/)" "200" "directory serves index"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P1/uploads/)" "200|403" "autoindex route"

# ---- Uploads ----
echo "[UPLOAD] file upload and retrieval"
rm -f www/uploads/2010/stress_*.bin
up_code=$(curl -s --max-time 10 -o /dev/null -w "%{http_code}" -F "file=@Makefile" http://$HOST:$P1/uploads)
expect "$up_code" "200|201|204" "multipart upload"
# try to retrieve the newest uploaded file
uploaded=$(ls -t www/uploads/2010/ | head -1)
if [ -n "$uploaded" ]; then
    expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" http://$HOST:$P1/uploads/2010/$uploaded)" "200" "retrieve uploaded file"
fi

# ---- CGI ----
echo "[CGI] PHP and Python execution"
expect "$(curl -s --max-time 15 -o /dev/null -w "%{http_code}" "http://$HOST:$P1/cgi-bin/test.php")" "200" "PHP CGI GET"
expect "$(curl -s --max-time 15 -o /dev/null -w "%{http_code}" -X POST -d "name=alice" http://$HOST:$P1/cgi-bin/test.php)" "200" "PHP CGI POST"
expect "$(curl -s --max-time 15 -o /dev/null -w "%{http_code}" http://$HOST:$P1/cgi-bin/song_request.py)" "200" "Python CGI GET"

# ---- Request parsing edge cases ----
echo "[REQUEST PARSING] malformed / edge-case requests"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" "http://$HOST:$P1/index%2Ehtml")" "200" "percent-encoded path"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" -X POST -H "Transfer-Encoding: chunked" -d '4\r\ntest\r\n5\r\nchunk\r\n0\r\n\r\n' http://$HOST:$P1/)" "200|201|204|400" "chunked POST (libcurl may buffer)"

# ---- Body size limit ----
echo "[BODY LIMIT] client_max_body_size"
# write body to file to avoid shell argument length limits
python3 -c "print('x' * (11 * 1024 * 1024))" > /tmp/bigbody.txt
expect "$(curl -s --max-time 15 -o /dev/null -w "%{http_code}" -X POST --data-binary @/tmp/bigbody.txt http://$HOST:$P1/)" "413" "oversized body rejected"

# ---- Connection management ----
echo "[CONNECTION] keep-alive / close"
expect "$(curl -s --max-time 5 -o /dev/null -w "%{http_code}" -H "Connection: close" http://$HOST:$P1/)" "200" "Connection: close handled"

# ---- Stress ----
echo "[STRESS] concurrent and repeated requests"
seq 1 50 | xargs -P 20 -I{} curl -s --max-time 10 -o /dev/null -w "%{http_code}\n" http://$HOST:$P1/ > /tmp/stress_2010.txt
ok_count=$(grep -c "^200$" /tmp/stress_2010.txt || true)
if [ "$ok_count" -eq 50 ]; then
    echo "  [PASS] 50 concurrent GETs all returned 200"
    pass=$((pass+1))
else
    echo "  [FAIL] 50 concurrent GETs: $ok_count/200 returned 200"
    fail=$((fail+1))
fi

for i in 1 2 3; do
    seq 1 30 | xargs -P 10 -I{} curl -s --max-time 10 -o /dev/null -w "%{http_code}\n" "http://$HOST:$P1/cgi-bin/test.php?n={}" > /tmp/stress_cgi_$i.txt
done
total_cgi=90
ok_cgi=$(grep -h "^200$" /tmp/stress_cgi_*.txt | wc -l || true)
if [ "$ok_cgi" -ge $((total_cgi * 75 / 100)) ]; then
    echo "  [PASS] $total_cgi concurrent CGI requests: $ok_cgi returned 200 (>= 75%)"
    pass=$((pass+1))
else
    echo "  [FAIL] $total_cgi concurrent CGI requests: $ok_cgi/$total_cgi returned 200 (need >= 75%)"
    fail=$((fail+1))
fi

# ---- Telnet raw tests ----
echo "[TELNET] raw HTTP/1.1 and malformed input"

telnet_status() {
    local payload="$1" want="$2" desc="$3"
    local out
    out=$(printf '%b' "$payload" | nc -q 2 $HOST $P1 2>/dev/null | head -1)
    if echo "$out" | grep -Eq "HTTP/1\.[01] ${want}"; then
        echo "  [PASS] $desc -> $out"
        pass=$((pass+1))
    else
        echo "  [FAIL] $desc -> $out (expected $want)"
        fail=$((fail+1))
    fi
}

telnet_status 'GET / HTTP/1.1\r\nHost: localhost\r\n\r\n' "200" "raw HTTP/1.1 GET"
telnet_status 'GET / HTTP/1.0\r\nHost: localhost\r\n\r\n' "200|505" "raw HTTP/1.0 GET"
telnet_status 'PUT / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n' "400|405|501" "raw unknown PUT"
telnet_status '\r\n' "400" "raw CRLF only"

echo "===== Results: $pass passed, $fail failed ====="
