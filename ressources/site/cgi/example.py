#!/usr/bin/env python3
import os

server_software = os.getenv("SERVER_SOFTWARE", "Unknown Server")

html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<title>Informations de l'Utilisateur</title>
	<style>
				@import url('https://fonts.googleapis.com/css2?family=Inter:ital,opsz,wght@0,14..32,100..900;1,14..32,100..900&display=swap');

		body {{
			font-family: 'Inter', sans-serif;
			background-color: #f0f0f0;
			display: flex;
			justify-content: center;
			align-items: center;
			height: 100vh;
			color: #333;
		}}
		.container {{
			text-align: center;
		}}
	</style>
</head>
<body>
	<div class="container">
		<h1>Bienvenue sur {server_software}!</h1>
	</div>
</body>
</html>
"""

# --- headers ---
print("Content-Type: text/html")
print("Server: HajServ/1.0")
print("Content-Length: {}".format(len(html_content)))
print("Cache-Control: no-cache")
print()  # End of headers

print(html_content)
