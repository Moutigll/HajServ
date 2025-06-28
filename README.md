# 🦈 HajServ - High-Performance HTTP Server in C++98

![HajServ](ressources/imgs/HajServ.png)

HajServ is a fully custom HTTP/1.1 web server written in C++98. It features support for multiple clients, CGI execution, static file serving, detailed logging, customizable configuration, and performance benchmarking tools.

---

## 📦 Features

- HTTP/1.1 server supporting **GET**, **POST**, **DELETE**
- Support for **multiple server blocks**
- **Virtual hosting** on different ports and domains
- **Custom error pages**
- **Static file serving** with directory auto-indexing
- **CGI script execution**
- **Alias & root directives**
- **Logging** to file and/or console
- **Request body size limiting**
- **Timeouts per connection**
- Optimized for performance

  ---

## 🛠️ Building the Server

### Requirements

- GNU Make
- g++ (compatible with C++98)
- Unix-like OS (Linux recommended)

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/Moutigll/HajServ.git
   cd HajServ
   ```

2. Compile the project:
   ```bash
   make
   ```

   This will build the server executable `./webserv` and place all object files under `obj/`.

3. Run the server with a config file
   ```bash
   ./webserv example.cfg
   ```

---

## ⚙️ Configuration Guide

The server reads a configuration file written in a simplified Nginx-style syntax.

### Example

```bash
log_level debug;
log_file ./server.log;
log_console on;

server {
	server_name localhost www.local.test;
	listen 8080;
	listen 4242;
	timeout 30;
	allowed_methods GET POST HEAD;
	max_body_size 4096;

	error_pages ./ressources/html/errors/ {
		404 404.html;
		500 500.html;
		502 502.html;
	}

	location / {
		root ./ressources/html/;
		index index.html;
		allowed_methods GET POST;
		autoindex on;
	}

	location /images/ {
		root ./ressources/images/;
		autoindex off;
	}
}
```

### Supported Directives

#### 🔹 Global Scope

- `log_level [debug|info|warning|error];`
- `log_file ./path/to/log;`
- `log_console [on|off];`

#### 🔹 server Block

- `server_name` – space-separated list of domains
- `listen` – port number (can be repeated)
- `timeout` – request timeout in seconds
- `allowed_methods` – subset of `GET POST HEAD`
- `max_body_size` – maximum allowed POST body size in bytes
- `error_pages` – folder and mapping of status code to file

#### 🔹 location Block

- `root` – file system root for this location
- `index` – default file to serve
- `allowed_methods` – restrict per route
- `autoindex` – show directory listing (`on` or `off`)

#### 🔹 CGI Support

To enable CGI for specific file extensions, use the `fastcgi_pass` directive inside a `location` block.

```bash
location /cgi/ {
	root ./ressources/site/cgi;

	# Bind interpreter to extension
	fastcgi_pass /bin/python3 .py;
	fastcgi_pass /bin/php .php;
	fastcgi_pass /bin/bash .sh;

	# Max execution time (in seconds)
	fastcgi_read_timeout 3;
}
```

- `fastcgi_pass` – binds an executable to a file extension
- `fastcgi_read_timeout` – limits execution time of CGI script

---

## Performance

### Benchmark executed locally (no logs, compiled with -O3)

The performance tests were run on a localhost with logging disabled, using the -O3 optimization flag during compilation.  

The Python script used to generate the graphs is located at:

```
ressources/scripts/graph.py
```

### Raw Results

- **High latency scenario**: includes all measurements, including outliers (extreme delays, GC, etc.).
- **Low latency scenario**: trimmed mean (1% of extreme values removed), giving a clearer view of real performance.

<p align="center">
  <img src="ressources/imgs/high.png" alt="High latency plot (with outliers)" width="45%" />
  <img src="ressources/imgs/low.png" alt="Low latency plot (1% extremes removed)" width="45%" />
</p>

### High (raw) latency data

```bash
Total requests:       564021
Successful:           563921
Failed:               100
Average time (ms):    2.72
Median time (ms):     0.35
Average requests/sec: 40560.07
Longest:              2789.93ms
Shortest:             0.09ms
```

### Low (filtered) latency data

```bash
Total requests:       571437
Successful:           571437
Failed:               0
Average time (ms):    0.34
Median time (ms):     0.34
Average requests/sec: 38097.74
Longest:              0.64ms
Shortest:             0.26ms
```

## 📜 License

This project is licensed under the GNU-3.0 License. See the `LICENSE` file for more details.
