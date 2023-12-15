#!/usr/bin/env python

import cgi

print("Content-Type: text/html")  # HTML is following
print()                           # Blank line, end of headers

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
name = form.getvalue('name', 'Unknown')
age = form.getvalue('age', 'Unknown')

# Generate the HTML content
print(f"<html><head><title>CGI Script</title></head>")
print(f"<body>")
print(f"<h1>CGI Script Response</h1>")
print(f"<p>Name: {name}</p>")
print(f"<p>Age: {age}</p>")
print(f"</body></html>")
