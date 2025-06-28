#!/bin/bash


# Contenu HTML
cat <<EOF
<!DOCTYPE html>
<html>
<head>
    <title>CGI Bash Test</title>
</head>
<body>
    <h1>Hello from Bash CGI!</h1>
    <p>Current server time: $(date)</p>
</body>
</html>
EOF
