#!/usr/bin/env python3

import datetime, io, base64
from PIL import Image, ImageDraw

def generate_fractal(width=600, height=600, max_iter=150):
    img = Image.new('RGB', (width, height))
    draw = ImageDraw.Draw(img)
    for x in range(width):
        for y in range(height):
            zx = (x - width/2) * 4 / width
            zy = (y - height/2) * 4 / width
            c = complex(zx, zy)
            z = 0+0j
            color = 0
            for i in range(max_iter):
                z = z*z + c
                if abs(z) > 2:
                    color = 255 - int(255 * i / max_iter)
                    break
            draw.point([x, y], (color, color, 255))
    return img

img = generate_fractal()
buf = io.BytesIO()
img.save(buf, format='PNG')
b64 = base64.b64encode(buf.getvalue()).decode('ascii')

now = datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S UTC')

# HTML response
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Python CGI Fractal</title>
    <meta name="description" content="Dynamic Mandelbrot fractal generated live!" />
    <meta name="date" content="{now}" />
    <style>
        body {{
            background: #222;
            color: #eee;
            font-family: monospace;
            text-align: center;
            padding: 20px;
        }}
        img {{
            border: 4px solid #eee;
            border-radius: 8px;
            width: 600px;
            height: 600px;
        }}
        .footer {{
            margin-top: 20px;
            font-size: 0.9em;
            color: #aaa;
        }}
    </style>
</head>
<body>
    <h1>Live Mandelbrot Fractal</h1>
    <p>Generated at: {now}</p>
    <img src="data:image/png;base64,{b64}" alt="Mandelbrot Fractal" />
    <div class="footer">A Python CGI demo on HajServ</div>
</body>
</html>
""")
