#!/bin/sh

# Simple CGI script that changes background color on button click

echo "Content-Type: text/html"
echo ""

# Read QUERY_STRING from environment (e.g. color=red)
COLOR=$(echo "$QUERY_STRING" | sed -n 's/^.*color=\([^&]*\).*$/\1/p')

# Define colors cycle
case "$COLOR" in
    red) NEXT_COLOR=green ;;
    green) NEXT_color=blue ;;
    blue) NEXT_COLOR=yellow ;;
    yellow) NEXT_COLOR=red ;;
    *) NEXT_COLOR=red ;;  # default color
esac

# If no color provided, default
if [ -z "$COLOR" ]; then
    COLOR="white"
    NEXT_COLOR="red"
fi

cat <<EOF
<html>
<head>
<title>Color Switcher</title>
</head>
<body style="background-color: $COLOR;">
<h1>Click the button to change background color</h1>
<form method="GET">
    <input type="hidden" name="color" value="$NEXT_COLOR" />
    <button type="submit">Change Color</button>
</form>
</body>
</html>
EOF
