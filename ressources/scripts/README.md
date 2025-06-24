# Siege Benchmark Visualizer (Custom C Implementation)

This project provides a lightweight benchmarking tool written in **C** that simulates siege-style load testing on websites. A Python script is provided to **generate graphs** from the collected response times.

Unlike the original [`siege`](https://www.joedog.org/siege-home/), this project is a **minimal custom version written entirely in C**, with full control over its behavior and output format.

---

## 📁 Project Structure

```bash
.
├── Siege.c                # C implementation of the benchmarking tool
├── siege                  # Compiled binary (output of Siege.c)
├── graph.py               # Python script to generate performance graphs
├── results.csv            # Automatically generated benchmark results
├── requirements.txt       # Python packages for graph.py
├── examples/              # Sample output images
```

---

## ⚙️ Installation & Setup

### 1. Install Python with pyenv (Recommended)

To avoid conflicts with system Python:

```bash
# Install pyenv if not already installed
curl https://pyenv.run | bash

# Restart your shell or follow pyenv instructions

# Install a specific Python version (e.g. 3.11)
pyenv install 3.11.9
pyenv local 3.11.9

# Install dependencies
pip install -r requirements.txt
```

---

### 2. Compile the C Benchmark Tool

This tool requires the `libcurl` development library. On Ubuntu/Debian, you can install it with:

```bash
sudo apt install libcurl4-openssl-dev
```

Then compile the benchmarking tool with pthread and curl:

```bash
gcc -o siege Siege.c -lcurl -lpthread
```

This will create the `./siege` binary.

---

## 🚀 Usage

### Benchmark a Target Website

```bash
./siege --url=https://example.com -t=42 -d=10
./siege --url=42.42.42.42
```

This performs numerous HTTP requests and writes the average timing data to `results.csv`.

---

### Benchmark a Local Server

Start a test server in another terminal
Then run the benchmark:

```bash
python graph.py http://localhost:8000
```

This will rune siege and create visual graphs based on `results.csv`.

---

## 📊 Example Output

Here are some example graphs from the `/examples` directory:

### Cloudflare dns

![Cloudflare 1.1.1.1](examples/Cloudflare-dns.png)

### Google.com  
![google.com](examples/google.com.png)

---

### Docker Hub  
![docker-1-95](examples/Docker.png)

---

### OSTJourney (self-hosted)  Nginx + lim req/s and custom burst
![OSTJourney](examples/OSTJourney.png)

---

## 🧪 Notes

- Every run overwrites `results.csv`. Back it up if you want to compare results.

---

## 📜 License

This project is released under the GPL 3.0 license

Feel free to use, fork, and improve!
