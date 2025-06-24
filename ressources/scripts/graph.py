import threading
import pycurl
import time
from io import BytesIO
import matplotlib.pyplot as plt

url = "http://localhost:8080/"
concurrent_threads = 100
duration_seconds = 15

response_times = []
lock = threading.Lock()

def send_request():
	start = time.time()
	buffer = BytesIO()

	c = pycurl.Curl()
	c.setopt(c.URL, url)
	c.setopt(c.WRITEDATA, buffer)
	c.setopt(c.NOPROGRESS, True)
	c.setopt(c.CONNECTTIMEOUT, 5)
	c.setopt(c.TIMEOUT, 5)

	try:
		c.perform()
		status = c.getinfo(pycurl.RESPONSE_CODE)
	except pycurl.error:
		status = 0
	finally:
		c.close()

	elapsed = (time.time() - start) * 1000  # ms
	with lock:
		response_times.append((elapsed, status))

def worker_loop(stop_time):
	while time.time() < stop_time:
		send_request()

threads = []
stop_time = time.time() + duration_seconds

for _ in range(concurrent_threads):
	t = threading.Thread(target=worker_loop, args=(stop_time,))
	t.start()
	threads.append(t)

for t in threads:
	t.join()

times = [x[0] for x in response_times]
statuses = [x[1] for x in response_times]

success = sum(1 for s in statuses if 200 <= s < 300)
fail = len(statuses) - success
avg = sum(times) / len(times) if times else 0
median = sorted(times)[len(times) // 2] if times else 0

print(f"Duration:             {duration_seconds} seconds")
print(f"Total requests:       {len(times)}")
print(f"Successful:           {success}")
print(f"Failed:               {fail}")
print(f"Average time (ms):    {avg:.2f}")
print(f"Median time (ms):     {median:.2f}")
print(f"Average requests/sec: {len(times) / duration_seconds:.2f}")
print(f"Longest:              {max(times):.2f}ms")
print(f"Shortest:             {min(times):.2f}ms")

# Trace tout les points
plt.figure(figsize=(12, 6))
plt.plot(times, marker='.', linestyle='None', markersize=1, alpha=0.6, label='Response Time (ms)')
plt.title("Python siege graph")
plt.xlabel("Request number")
plt.ylabel("Time (ms)")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
