#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html\r\n")
print("""
    <!DOCTYPE html>
    <html lang='en'>
    <head>
        <meta charset='UTF-8'>
        <title>Env Variables Test</title>
    </head>
	<body>""")

print("== CGI Redirection Test ==<br>")
print(f"Current working dir: {os.getcwd()}<br>")
print(f"Script path: {os.path.abspath(sys.argv[0])}<br><br>")

print("== Environment Variables ==<br>")
for key, value in sorted(os.environ.items()):
	print(f"{key}: {value}<br>")
print("""</body>
</html>""")
