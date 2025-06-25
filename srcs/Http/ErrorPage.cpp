/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPage.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:51:57 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/26 01:09:14 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/HttpResponse.hpp"


#define SVG_CONTENT  \
"<path fill=\"url(#paint0_linear_919_109)\" d=\"M249.385 200.957C260.635 186.81 255.75 160.118 247.937 115.697C240.759 74.889 231.129 27.4763 174.268 30.1046C144.198 31.4944 119.179 37.0074 98.366 45.2855C98.366 45.2855 147.479 30.1046 169.803 55.0239C192.127 79.9429 169.803 116.435 169.803 155.784C169.803 200.206 191.011 222.53 191.011 222.53C220.951 218.401 238.393 214.779 249.385 200.957Z\"/>\n\
<script xmlns=\"\"/>\n\
<path fill=\"url(#paint1_linear_919_109)\" d=\"M12.9086 206.734C28.8256 221.081 68.335 223.175 90.0528 224.116C124.543 225.609 160.623 226.721 191.011 222.53C191.011 222.53 169.803 200.206 169.803 155.784C169.803 116.435 192.127 79.9429 169.803 55.0239C147.479 30.1046 98.366 45.2855 98.366 45.2855C55.715 62.2485 30.7193 90.8221 16.0923 119.321C-6.6571 163.646 -3.00846 192.388 12.9086 206.734Z\"/>\n\
<path fill=\"#192437\" d=\"M211.212 172.554C224.095 172.554 234.539 162.11 234.539 149.227C234.539 136.344 224.095 125.9 211.212 125.9C198.329 125.9 187.886 136.344 187.886 149.227C187.886 162.11 198.329 172.554 211.212 172.554Z\"/>\n\
<path xmlns=\"http://www.w3.org/2000/svg\" stroke=\"#192437\" stroke-width=\"12\" stroke-linecap=\"round\" d=\"M 48.236 135.062 C 68.841 92.394 142.242 112.275 134.171 151.863\"/>\n\
<path fill=\"#64A3E5\" fill-opacity=\"0.253286\" d=\"M131.477 40.7157C129.524 40.6899 127.592 40.7218 125.709 40.8069C123.825 40.8921 121.983 41.0296 120.205 41.1987C116.653 41.5364 113.345 42.0183 110.437 42.529C108.299 42.9048 106.657 43.2633 105.006 43.6316C101.039 44.5166 98.3637 45.29 98.3637 45.29C90.2385 48.5215 82.7603 52.1762 75.8662 56.1697C89.1213 53.3004 107.73 49.9041 121.608 49.9644C123.186 49.9713 124.701 50.0259 126.137 50.1284C139.851 51.1079 154.585 52.0288 162.849 61.2542C171.469 70.8763 171.612 82.209 168.963 98.4949C166.315 114.781 160.471 134.343 160.471 155.782C160.471 179.478 166.16 197.604 171.98 209.825C172.556 211.036 176.581 217.867 180.08 223.794C183.791 223.44 187.448 223.019 191.015 222.527C191.015 222.527 190.685 222.179 190.103 221.48C189.523 220.782 188.692 219.738 187.698 218.345C186.206 216.253 184.344 213.383 182.385 209.725C181.733 208.507 181.071 207.208 180.408 205.816C177.094 198.856 173.78 189.728 171.761 178.434C171.357 176.179 171.003 173.837 170.713 171.409V171.4C170.423 168.969 170.194 166.459 170.039 163.855C169.883 161.252 169.802 158.558 169.802 155.782C169.802 153.323 169.89 150.873 170.048 148.438C170.364 143.571 170.964 138.757 171.715 134.013V134.004C172.091 131.631 172.509 129.279 172.945 126.943C174.689 117.599 176.78 108.568 178.176 99.9893C178.873 95.7001 179.397 91.5252 179.615 87.4785C179.834 83.4287 179.748 79.5151 179.224 75.7422C179.093 74.7997 178.928 73.8688 178.741 72.9449C178.365 71.0943 177.882 69.2868 177.255 67.5141C177.167 67.2645 177.048 67.0241 176.955 66.776C176.385 65.2652 175.74 63.7796 174.959 62.3294C174.504 61.4831 174.016 60.6485 173.483 59.8236C172.952 59.0007 172.382 58.1917 171.77 57.3907C171.158 56.5896 170.499 55.8003 169.802 55.0216C169.104 54.2428 168.381 53.5095 167.633 52.8074C166.138 51.4032 164.544 50.1438 162.877 49.0259C162.043 48.4669 161.195 47.9419 160.325 47.4495C158.586 46.4646 156.778 45.6099 154.922 44.8708C153.991 44.5002 153.05 44.1642 152.097 43.8503C151.815 43.7572 151.525 43.6834 151.241 43.5951C149.845 43.1618 148.431 42.7803 147.004 42.4561C146.769 42.4028 146.538 42.3336 146.302 42.283H146.293C146.138 42.2499 145.983 42.2238 145.828 42.1919C143.038 41.6158 140.207 41.2214 137.39 40.9891C135.407 40.8255 133.429 40.7415 131.477 40.7157Z\"/>\n\
<defs>\n\
<linearGradient id=\"paint0_linear_919_109\" x1=\"173.913\" y1=\"28.6833\" x2=\"224.522\" y2=\"219.125\" gradientUnits=\"userSpaceOnUse\">\n\
<stop stop-color=\"#81B6ED\"/>\n\
<stop offset=\"1\" stop-color=\"#1D75D2\"/>\n\
</linearGradient>\n\
<linearGradient id=\"paint1_linear_919_109\" x1=\"102.014\" y1=\"47.3563\" x2=\"105.042\" y2=\"216.699\" gradientUnits=\"userSpaceOnUse\">\n\
<stop stop-color=\"#FAFAFA\"/>\n\
<stop offset=\"1\" stop-color=\"#E6E6E6\"/>\n\
</linearGradient>\n\
</defs>"

void	HttpResponse::buildErrorPage()
{
	_body.clear();
	_body += "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
	_body += "\t<meta charset=\"UTF-8\" />\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n";
	_body += "\t<title>" + to_string(_status) + " - " + _ErrorStatus.getMessage(_status) + "</title>\n\t<style>\n";
	_body += "\t\tbody {\n\t\t\tmargin: 0;\n\t\t\tpadding: 0;\n\t\t\tbackground: #4F6EA5;\n\t\t\toverflow: hidden;\n\t\t\tfont-family: Arial, sans-serif;\n\t\t\tcolor: #bce3ff;\n\t\t}\n";
	_body += "\t\tsection {\n\t\t\tposition: relative;\n\t\t\twidth: 100%;\n\t\t\theight: 100vh;\n\t\t\ttext-align: center;\n\t\t\tdisplay: flex;\n\t\t\tflex-direction: column;\n\t\t\tjustify-content: center;\n\t\t\talign-items: center;\n\t\t\tz-index: 10;\n\t\t}\n";
	_body += "\t\tsvg.blahaj {\n\t\t\twidth: 250px;\n\t\t\theight: auto;\n\t\t\tmargin-bottom: 20px;\n\t\t\tz-index: 1000;\n\t\t}\n";
	_body += "\t\th1 {\n\t\t\tfont-size: 5rem;\n\t\t\tletter-spacing: 15px;\n\t\t\tcolor: #bce3ff;\n\t\t\tfont-family: Impact, Haettenschweiler, \"Arial Narrow Bold\", sans-serif;\n\t\t\tmargin: 0;\n\t\t}\n";
	_body += "\t\th2 {\n\t\t\tfont-size: 2rem;\n\t\t\tmargin: 10px 0;\n\t\t\tcolor: #8ac4ff;\n\t\t}\n";
	_body += "\t\th3 {\n\t\t\tfont-size: 1rem;\n\t\t\tmargin-top: 5px;\n\t\t\tcolor: #5ca0d3;\n\t\t}\n";
	_body += "\t\th1, h2, h3 { z-index: 1000; }\n";
	_body += "\t\t.wave {\n\t\t\tposition: absolute;\n\t\t\tbottom: 0;\n\t\t\tleft: 0;\n\t\t\twidth: 200%;\n\t\t\theight: 200px;\n\t\t\tanimation: waveAnimation 10s linear infinite;\n\t\t\tz-index: 0;\n\t\t}\n";
	_body += "\t\t@keyframes waveAnimation {\n\t\t\t0% { transform: translateX(0); }\n\t\t\t100% { transform: translateX(-50%); }\n\t\t}\n";
	_body += "\t\t.wave1 { fill: #173661; animation-duration: 18s; z-index: 8; bottom: -1vh; }\n";
	_body += "\t\t.wave2 { fill: #193B6A; animation-duration: 16s; z-index: 7; bottom: 3vh; }\n";
	_body += "\t\t.wave3 { fill: #1C3F72; animation-duration: 14s; z-index: 6; bottom: 9vh; }\n";
	_body += "\t\t.wave4 { fill: #1F447B; animation-duration: 13s; z-index: 5; bottom: 16vh; }\n";
	_body += "\t\t.wave5 { fill: #224984; animation-duration: 10s; z-index: 4; bottom: 21vh; }\n";
	_body += "\t\t.wave6 { fill: #2E528C; animation-duration: 9s; z-index: 3; bottom: 28vh; }\n";
	_body += "\t\t.wave7 { fill: #3A5B94; animation-duration: 8s; z-index: 2; bottom: 35vh; }\n";
	_body += "\t\t.wave8 { fill: #44659D; animation-duration: 7s; z-index: 1; bottom: 39vh; }\n";
	_body += "\t</style>\n</head>\n<body>\n<section>\n";
	_body += "\t<svg class=\"blahaj\" width=\"256\" height=\"256\" viewBox=\"0 0 256 256\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">\n";
	_body += "\t\t" + to_string(SVG_CONTENT) + "</svg>\n";
	_body += "\t<h1> Error " + to_string(_status) + "</h1>\n\t<h2>" + _ErrorStatus.getMessage(_status) + "</h2>\n\t<h3>" + std::string(VERSION) + "</h3>\n";
	for (int i = 8; i >= 1; --i)
	{
		_body += "\t<svg class=\"wave wave" + to_string(i) + "\" viewBox=\"0 0 1200 300\" preserveAspectRatio=\"none\" shape-rendering=\"geometricPrecision\">\n";
		_body += "\t\t<path d=\"M0 150C100 200 200 200 300 150S500 100 600 150 800 200 900 150 1100 100 1200 150L1200 300 0 300Z\" />\n\t</svg>\n";
	}
	_body += "</section>\n</body>\n</html>\n";

	_headers["Content-Type"] = "text/html";
	g_logger.log(LOG_WARNING, "No error page defined for status code " + to_string(_status));
}

