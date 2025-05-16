import socket
import concurrent.futures

HOST = '127.0.0.1'
PORTS = [8080, 9090, 10000]
TIMEOUT = 3

REQUESTS = [
	b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",                # 200 OK
	b"GET  HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",                # 400 Bad Request (malformed URI)
	b"BADMETHOD / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",         # 400 Bad Request (invalid method)
	b"GET / \r\nHost: localhost\r\nConnection: close\r\n\r\n",                       # 400 Bad Request (malformed request line)
	b"GET / HTTP/1.1\r\nHost localhost\r\nConnection: close\r\n\r\n",                # 400 Bad Request (missing colon in Host)
	b"HELLO\r\n\r\n",                                                                # 400 Bad Request (invalid request)
	b"GET / HTTP/1.1\r\nConnection: close\r\n\r\n",                                  # 400 Bad Request (missing Host header)
	b"OPTIONS / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",           # 405 Method Not Allowed
	b"GET / HTTP/2.0\r\nHost: localhost\r\nConnection: close\r\n\r\n",               # 505 HTTP Version Not Supported
	b"POST / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",              # 501 Not Implemented (only GET supported)
]

EXPECTED_CODES = [
	"200",
	"400",
	"400",
	"400",
	"400",
	"400",
	"400",
	"405",
	"505",
	"501",
]

GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"

def test_request(port, request_bytes, id_test, expected_code, request_index):
	try:
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.settimeout(TIMEOUT)
		sock.connect((HOST, port))
		sock.sendall(request_bytes)

		data = sock.recv(1024)
		sock.close()
		if not data:
			return (id_test, port, "NO_RESPONSE", expected_code, request_index)

		lines = data.split(b"\r\n")
		if len(lines) > 0:
			status_line = lines[0].decode(errors='ignore')
			parts = status_line.split(' ')
			if len(parts) >= 2:
				code = parts[1]
				return (id_test, port, code, expected_code, request_index)
		return (id_test, port, "UNKNOWN_RESPONSE", expected_code, request_index)
	except socket.timeout:
		return (id_test, port, "TIMEOUT", expected_code, request_index)
	except Exception as e:
		return (id_test, port, f"ERROR: {e}", expected_code, request_index)

def main():
	with concurrent.futures.ThreadPoolExecutor(max_workers=20) as executor:
		futures = []
		test_id = 0
		for port in PORTS:
			for i, request in enumerate(REQUESTS):
				expected = EXPECTED_CODES[i]
				for _ in range(2):
					futures.append(executor.submit(test_request, port, request, test_id, expected, i))
					test_id += 1

		total_tests = 0
		passed_tests = 0

		for future in concurrent.futures.as_completed(futures):
			id_test, port, result, expected, request_index = future.result()
			request_preview = REQUESTS[request_index].decode(errors='ignore').split('\r\n')[0]
			color = GREEN if result == expected else RED
			print(f"[Test {id_test:03}][Port {port}][Req {request_index} '{request_preview}'] Result: {result} | Expected: {expected} {color}{'OK' if result == expected else 'KO'}{RESET}")
			
			total_tests += 1
			if result == expected:
				passed_tests += 1

		percentage = (passed_tests / total_tests) * 100 if total_tests > 0 else 0
		print(f"\nSummary: Passed {passed_tests} / {total_tests} tests ({percentage:.2f}%)")

if __name__ == "__main__":
	main()
