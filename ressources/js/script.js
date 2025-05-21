function getRandomColorHex() {
	const letters = "0123456789ABCDEF";
	let color = "#";
	for (let i = 0; i < 6; i++) {
		color += letters[Math.floor(Math.random() * 16)];
	}
	return color;
}


document.addEventListener('DOMContentLoaded', () => {
	const button = document.getElementById('toggleButton');
	let toggled = false;

	button.addEventListener('click', () => {
		document.body.style.backgroundColor = getRandomColorHex();
		toggled = !toggled;
	});
});
