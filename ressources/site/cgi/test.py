#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/plain\r\n")

print("== CGI Redirection Test ==")
print(f"Current working dir: {os.getcwd()}")
print(f"Script path: {os.path.abspath(sys.argv[0])}\n")

print("== Environment Variables ==")
for key, value in sorted(os.environ.items()):
	print(f"{key}: {value}")
