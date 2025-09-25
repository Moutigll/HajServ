#!/usr/bin/env python3

import cgi
import cgitb
import os

cgitb.enable()  # Enable debugging

FILE_PATH = "../user-data/post"

def save_message(name, email, message):
    with open(FILE_PATH, "a") as f:
        f.write(f"Name: {name}\nEmail: {email}\nMessage: {message}\n{'-'*40}\n")

def show_form():
    print("Content-Type: text/html")
    print()
    print("""
    <!DOCTYPE html>
    <html lang='en'>
    <head>
        <meta charset='UTF-8'>
        <title>Contact Form</title>
    </head>
    <body>
        <h1>Contact Us</h1>
        <form method="post" action="">
            <label>Name: <input type="text" name="name" required></label><br><br>
            <label>Email: <input type="email" name="email" required></label><br><br>
            <label>Message:<br>
                <textarea name="message" rows="5" cols="40" required></textarea>
            </label><br><br>
            <input type="submit" value="Send">
        </form>
    </body>
    </html>
    """)

def show_thank_you(name, email, message):
    print("Content-Type: text/html")
    print()
    print(f"""
    <!DOCTYPE html>
    <html lang='en'>
    <head>
        <meta charset='UTF-8'>
        <title>Thanks</title>
    </head>
    <body>
        <h1>Thank you, {name}!</h1>
        <p>We have received your message:</p>
        <blockquote>{message}</blockquote>
        <p>We will contact you at: {email}</p>
        <p><a href="">Back to form</a></p>
    </body>
    </html>
    """)

def main():
    form = cgi.FieldStorage()
    if "name" in form and "email" in form and "message" in form:
        name = form.getfirst("name", "")
        email = form.getfirst("email", "")
        message = form.getfirst("message", "")
        save_message(name, email, message)
        show_thank_you(name, email, message)
    else:
        show_form()

if __name__ == "__main__":
    main()
