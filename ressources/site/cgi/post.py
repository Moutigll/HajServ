#!/usr/bin/env python3

import os
import sys
import urllib.parse

def main():
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.read(content_length)
    data = urllib.parse.parse_qs(post_data)

    name = data.get('name', [''])[0]
    email = data.get('email', [''])[0]
    message = data.get('message', [''])[0]

    print("Content-Type: text/html")
    print()
    print("<!DOCTYPE html>")
    print("<html lang='en'>")
    print("<head><meta charset='UTF-8'><title>Thanks</title></head>")
    print("<body>")
    print(f"<h1>Thank you, {name}!</h1>")
    print("<p>We have received your message:</p>")
    print(f"<blockquote>{message}</blockquote>")
    print(f"<p>We will contact you at: {email}</p>")
    print("</body></html>")

if __name__ == "__main__":
    main()
