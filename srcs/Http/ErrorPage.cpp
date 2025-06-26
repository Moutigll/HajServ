/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPage.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:51:57 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/26 03:06:29 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/HttpResponse.hpp"

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
	_body += "\t<svg class=\"blahaj\" width=\"256\" height=\"256\" viewBox=\"0 0 570 570\" fill=\"none\">\n";
	_body += std::string("\t\t") + svgBlahajBase + svgBlahajEye + svgBlahajTummy + svgBlahajShadow + svgBlahajLinearGradients + svgBlahajSad + "</svg>\n";
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

