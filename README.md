# 🌐 Webserv - HTTP Server in C++98
---
[![CMake Build and Test with Valgrind and Siege](https://github.com/Moutigll/webserv/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/Moutigll/webserv/actions/workflows/c-cpp.yml)
## Running the Server

### 1. Compile

```bash
make
```

### 2. Run the server

With a configuration file:

```bash
./webserv config/default.conf
```

Without arguments: the `default.conf` file will be used automatically.

---

## ⚙️ Configuration File Structure

A minimal example:

```conf
server {
    server_name Server1;
    listen 8080;
}
```

Multiple server blocks can be defined in the configuration file to listen on different ports or domains.

---

## 🧪 Testing with Telnet

### 1. Start the server (for example on 127.0.0.1:8080)

```bash
./webserv config/my_servers.conf
```

### 2. Connect with telnet

```bash
telnet 127.0.0.1 8080
```
