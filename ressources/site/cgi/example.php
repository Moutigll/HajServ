<?php
// Simple CGI PHP script

echo "Content-Type: text/html\r\n";
echo "\r\n"; // End of headers
echo "<html><head><title>CGI Test</title></head><body>";
echo "<h1>Hello from PHP CGI script!</h1>";
echo "<p>The current server time is: " . date('Y-m-d H:i:s') . "</p>";
echo "<p>Request method: " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>Query string: " . htmlspecialchars($_SERVER['QUERY_STRING']) . "</p>";
echo "</body></html>";
?>
