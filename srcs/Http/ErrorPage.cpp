/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPage.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:51:57 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/29 03:20:25 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/GetFiles.hpp"
#include "../../includes/Logger.hpp"
#include "../../includes/Http/HttpResponse.hpp"

void HttpResponse::buildErrorPage()
{
	_filePath = _ErrorStatus.getFilePath();
	if (!_filePath.empty() && access(_filePath.c_str(), R_OK) == 0)
		return;
	_body.clear();
	_body = "<!DOCTYPE html>\n"
			"<html lang=\"en\">\n"
			"<head>\n"
			"<meta charset=\"UTF-8\"/>\n"
			"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\"/>\n"
			"<title>" + to_string(_status) + " - " + _ErrorStatus.getMessage(_status) + "</title>\n"
			"<style>\n"
			"body{margin:0;padding:0;background:#4F6EA5;overflow:hidden;font-family:Arial,sans-serif;color:#bce3ff}\n"
			"section{position:relative;width:100%;height:100vh;text-align:center;display:flex;flex-direction:column;justify-content:center;align-items:center;z-index:10}\n"
			"svg.bl{width:250px;height:auto;margin-bottom:20px;z-index:1000}\n"
			"h1{font-size:5rem;letter-spacing:15px;color:#bce3ff;font-family:Impact,Haettenschweiler,\"Arial Narrow Bold\",sans-serif;margin:0}\n"
			"h2{font-size:2rem;margin:10px 0;color:#8ac4ff}\n"
			"h3{font-size:1rem;margin-top:5px;color:#5ca0d3}\n"
			"h1,h2,h3{z-index:1000}\n"
			".w{position:absolute;bottom:0;left:0;width:200%;height:200px;animation:wA 10s linear infinite;z-index:0}\n"
			"@keyframes wA{0%{transform:translateX(0)}100%{transform:translateX(-50%)}}\n"
			".w1{fill:#173661;animation-duration:18s;z-index:8;bottom:-1vh}\n"
			".w2{fill:#193B6A;animation-duration:16s;z-index:7;bottom:3vh}\n"
			".w3{fill:#1C3F72;animation-duration:14s;z-index:6;bottom:9vh}\n"
			".w4{fill:#1F447B;animation-duration:13s;z-index:5;bottom:16vh}\n"
			".w5{fill:#224984;animation-duration:10s;z-index:4;bottom:21vh}\n"
			".w6{fill:#2E528C;animation-duration:9s;z-index:3;bottom:28vh}\n"
			".w7{fill:#3A5B94;animation-duration:8s;z-index:2;bottom:35vh}\n"
			".w8{fill:#44659D;animation-duration:7s;z-index:1;bottom:39vh}\n"
			"</style></head><body><section>\n"
			"<svg class=\"bl\" width=\"256\" height=\"256\" viewBox=\"0 0 570 570\" fill=\"none\">" + 
			svgBlahajBase + svgBlahajEye + svgBlahajTummy + svgBlahajShadow + 
			svgBlahajLinearGradients + svgBlahajSad + "</svg>\n"
			"<h1>Error " + to_string(_status) + "</h1>\n"
			"<h2>" + _ErrorStatus.getMessage(_status) + "</h2>\n"
			"<h3>" + std::string(VERSION) + "</h3>\n";

	for (int i = 8; i >= 1; --i) {
		_body += "<svg class=\"w w" + to_string(i) + "\" viewBox=\"0 0 1200 300\" preserveAspectRatio=\"none\" shape-rendering=\"geometricPrecision\">\n"
				"<path d=\"M0 150C100 200 200 200 300 150S500 100 600 150 800 200 900 150 1100 100 1200 150L1200 300 0 300Z\"/></svg>";
	}

	_body += "</section></body></html>";

	_headers["Content-Type"] = "text/html";
	g_logger.log(LOG_WARNING, "No error page defined for status code " + to_string(_status));
}
