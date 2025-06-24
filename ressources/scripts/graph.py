import sys
import subprocess
import time
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import os

def run_siege(url):
	cmd = [
		"./siege",
		f"--url={url}",
	]


	print(f"Launching siege on: {url}")
	process = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

	for i in range(16, 0, -1):
		print(f"Remaining: ~{i} seconds", end='\r')
		time.sleep(1)

	process.wait()
	print("\nSiege complete.\n")


def plot_results(filename, low=False):
	if not os.path.exists(filename):
		print(f"File not found: {filename}")
		return

	data = np.genfromtxt(filename, delimiter=',', skip_header=1)
	if data.ndim < 2 or data.shape[1] < 3:
		print("Invalid CSV structure")
		return

	x_start_us = data[:, 0]
	y_delay = data[:, 1] / 1000.0  # microseconds → milliseconds
	statuses = data[:, 2].astype(int)

	if low:
		low_percentile = np.percentile(y_delay, 1)
		high_percentile = np.percentile(y_delay, 99)
		mask = (y_delay >= low_percentile) & (y_delay <= high_percentile)

		x_start_us = x_start_us[mask]
		y_delay = y_delay[mask]
		statuses = statuses[mask]

	x_start_ms = (x_start_us - np.min(x_start_us)) / 1000.0

	request_ids = np.arange(1, len(x_start_ms) + 1)


	success = np.sum((statuses >= 200))
	fail = len(statuses) - success
	avg = np.mean(y_delay)
	median = np.median(y_delay)

	duration_seconds = x_start_ms[-1] / 1000.0 if len(x_start_ms) > 0 else 1

	print(f"Duration:             {duration_seconds:.2f} seconds")
	print(f"Total requests:       {len(y_delay)}")
	print(f"Successful:           {success}")
	print(f"Failed:               {fail}")
	print(f"Average time (ms):    {avg:.2f}")
	print(f"Median time (ms):     {median:.2f}")
	print(f"Average requests/sec: {len(y_delay) / duration_seconds:.2f}")
	print(f"Longest:              {np.max(y_delay):.2f}ms")
	print(f"Shortest:             {np.min(y_delay):.2f}ms")

	hist, bin_edges = np.histogram(x_start_ms, bins=200, density=True)
	bin_centers = 0.5 * (bin_edges[1:] + bin_edges[:-1])
	density_interp = np.interp(x_start_ms, bin_centers, hist)
	norm_density = (density_interp - density_interp.min()) / (density_interp.max() - density_interp.min())

	dpi = 101
	fig = plt.figure(figsize=(1920 / dpi, 1080 / dpi), dpi=dpi)
	gs = gridspec.GridSpec(1, 2, width_ratios=[20, 1], wspace=0.05)

	ax1 = fig.add_subplot(gs[0])
	# Mask for successes and failures
	mask_success = statuses >= 200
	mask_fail = ~mask_success


	sc = ax1.scatter(
		x_start_ms[mask_success],
		y_delay[mask_success],
		c=norm_density[mask_success],
		cmap='plasma',
		s=20,
		alpha=0.8,
		label="Success"
	)
	sc_fail = ax1.scatter(
		x_start_ms[mask_fail],
		y_delay[mask_fail],
		c=norm_density[mask_fail],
		cmap='plasma',
		s=20,
		alpha=0.8,
		label="Failed (normal color)"
	)

	ax1.scatter(
		x_start_ms[mask_fail],
		y_delay[mask_fail],
		facecolors='red',
		marker='.',
		s=1,
		alpha=0.9,
		label="Failed (red border)"
	)

	ax1.set_xlabel("Start time since beginning (ms)")
	ax1.set_ylabel("Response Time (ms)")
	ax1.grid(True)

	id_ticks = np.linspace(1, len(request_ids), num=11, dtype=int)
	id_pos = x_start_ms[id_ticks - 1]

	ax2 = ax1.twiny()
	ax2.set_xlim(ax1.get_xlim())
	ax2.set_xticks(id_pos)
	ax2.set_xticklabels(id_ticks)
	ax2.set_xlabel("Request ID")

	cax = fig.add_subplot(gs[1])
	cbar = plt.colorbar(sc, cax=cax)
	cbar.ax.yaxis.set_label_position('right')
	cbar.ax.yaxis.tick_right()
	cbar.set_label("Request density")

	plt.suptitle("Request delay vs start time (color = density)")
	plt.tight_layout(rect=[0, 0, 1, 0.95])
	fig.subplots_adjust(left=0.08, right=0.94)
	plt.savefig("siege.png", dpi=dpi, bbox_inches='tight')
	plt.show()


def main():
	if len(sys.argv) < 2:
		print("Usage: python3 siege_graph.py <url>")
		sys.exit(1)

	url = sys.argv[1]
	low = True if len(sys.argv) > 2 and sys.argv[2].lower() == 'low' else False
	result_file = "results.csv"

	run_siege(url)
	plot_results(result_file, low)

if __name__ == "__main__":
	main()
