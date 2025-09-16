#!/usr/bin/env python3

import os
from http.cookies import SimpleCookie

print("Content-Type: text/html")  # HTTP header

file = open("/tmp/cookie.log", "a+")
file.seek(0)
try:
    num = int(file.read())
except:
    num = 0
file.close()


# Parse cookies from environment
cookie_header = os.environ.get("HTTP_COOKIE", "")
cookies = SimpleCookie()
cookies.load(cookie_header)

# Check if our cookie exists
if "mycookie" in cookies:
    message = f"Welcome back visitor #{cookies['mycookie'].value}! There have been {num} visitors so far."
    print()  # End headers
    print(f"<html><body><h1>{message}</h1></body></html>")
else:
    # Set a new cookie
    new_cookie = SimpleCookie()
    new_cookie["mycookie"] = f"cookie_value_{num + 1}"
    new_cookie["mycookie"]["path"] = "/cgi"
    file = open("/tmp/cookie.log", "w")
    file.write(str(num + 1))
    file.close()
    print(new_cookie.output())  # Set-Cookie header
    print()  # End headers
    print("<html><body><h1>Hello! Setting your cookie now.</h1></body></html>")
