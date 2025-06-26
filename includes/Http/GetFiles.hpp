/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetFiles.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 02:24:15 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/26 03:16:59 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETFILES_HPP
# define GETFILES_HPP

# include <dirent.h>

# include "../Utils.hpp"

/**
 * @brief Finds the best matching location for a given URI in a server's configuration.
 * 
 * @param server the server configuration to search within
 * @param uri the URI to match against the server's locations
 * @return t_location* pointer to the best matching location, or NULL if none found
 */
t_location	*findBestLocation(t_server *server, const std::string &uri);

/**
 * @brief Checks if the given HTTP method is allowed for the specified location.
 * 
 * @param method the HTTP method to check (e.g., "GET", "POST")
 * @param allowed_methods a vector of allowed methods for the location
 * @return
 */
bool		checkMethod(const std::string &method, const std::vector<std::string> &allowed_methods);

/**
 * @brief Generates an autoindex HTML page for a directory.
 * This function creates an HTML page listing the contents of a directory,
 * including links to files and subdirectories.
 * @param uri the URI of the directory being indexed
 * @param directory_path the path to the directory to index
 * @return std::string an HTML string representing the autoindex page
 * 
 */
std::string	generateAutoindexPage(const std::string &uri, const std::string &directory_path);

t_location	*GetCgi(t_server *server, const std::string &uri);

static const std::string svgIconFolder =
	"<svg class=\"icon\" viewBox=\"0 0 24 24\" width=\"24\" height=\"24\">\n"
	"\t<g transform=\"translate(0 -1028.4)\">\n"
	"\t\t<path d=\"m2 1033.4c-1.1046 0-2 0.9-2 2v14c0 1.1 0.89543 2 2 2h20c1.105 0 2-0.9 2-2v-14c0-1.1-0.895-2-2-2h-20z\" fill=\"#2980b9\"/>\n"
	"\t\t<path d=\"m3 1029.4c-1.1046 0-2 0.9-2 2v14c0 1.1 0.8954 2 2 2h11 5 2c1.105 0 2-0.9 2-2v-9-3c0-1.1-0.895-2-2-2h-2-5-1l-3-2h-7z\" fill=\"#2980b9\"/>\n"
	"\t\t<path d=\"m23 1042.4v-8c0-1.1-0.895-2-2-2h-11-5-2c-1.1046 0-2 0.9-2 2v8h22z\" fill=\"#bdc3c7\"/>\n"
	"\t\t<path d=\"m2 1033.4c-1.1046 0-2 0.9-2 2v6 1 6c0 1.1 0.89543 2 2 2h20c1.105 0 2-0.9 2-2v-6-1-6c0-1.1-0.895-2-2-2h-20z\" fill=\"#3498db\"/>\n"
	"\t</g>\n"
	"</svg>\n";

static const std::string svgIconFile =
	"<svg class=\"icon\" fill=\"#3498db\" viewBox=\"0 0 24 24\" width=\"24\" height=\"24\">"
	"<path d=\"M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z\"/>"
	"<polyline points=\"14 2 14 8 20 8\" fill=\"#2980b9\"/>"
	"</svg>";

/*-------------------
		Blåhaj
---------------------*/

static const std::string svgBlahajBase =
"<path fill=\"url(#paint0_linear_1016_2)\" d=\"M555.27 447.444C580.32 415.944 569.444 356.513 552.048 257.606C536.065 166.745 514.624 61.1777 388.018 67.0297C321.065 70.1243 265.36 82.3992 219.018 100.831C219.018 100.831 328.371 67.0297 378.077 122.514C427.782 177.998 378.077 259.249 378.077 346.863C378.077 445.77 425.297 495.478 425.297 495.478C491.961 486.284 530.797 478.218 555.27 447.444Z\"/>";

static const std::string svgBlahajTummy =
"<path fill=\"url(#paint1_linear_1016_2)\" d=\"M28.7417 460.307C64.1819 492.25 152.152 496.913 200.508 499.007C277.303 502.333 357.637 504.808 425.297 495.477C425.297 495.477 378.077 445.77 378.077 346.863C378.077 259.249 427.782 177.998 378.077 122.514C328.371 67.0296 219.018 100.831 219.018 100.831C124.053 138.6 68.3985 202.221 35.8306 265.676C-14.8225 364.367 -6.69853 428.364 28.7417 460.307Z\"/>";

static const std::string svgBlahajMouth =
"<ellipse cy=\"243.743\" transform=\"rotate(5.36028 217.056 243.743)\" fill=\"#F8F8F8\" cx=\"217.056\" rx=\"139.5\" ry=\"66\"/>";

static const std::string svgBlahajEye =
"<path fill=\"#192437\" d=\"M470.278 384.201C498.962 384.201 522.216 360.948 522.216 332.263C522.216 303.578 498.962 280.325 470.278 280.325C441.593 280.325 418.339 303.578 418.339 332.263C418.339 360.948 441.593 384.201 470.278 384.201Z\"/>";

static const std::string svgBlahajSad =
"<path stroke=\"#192437\" stroke-width=\"26\" stroke-linecap=\"round\" d=\"M75.4136 301.1612C128.9866 190.2244 319.8292 241.915 298.8446 344.8438\"/>\n";

static const std::string svgBlahajSmug =
"<path stroke=\"#192437\" stroke-width=\"26\" stroke-linecap=\"round\" d=\"M113.376 248.521C114.461 294.087 124.321 304.201 140.243 308.045C156.166 311.889 173.681 292.958 203.803 298.826C233.924 304.693 259.242 376.893 306.962 276.732\"/>";

static const std::string svgBlahajTongue =
"<path fill=\"url(#paint2_linear_1016_2)\" d=\"M160.175 312.341L188.623 302.987C192.749 301.63 197.275 302.345 200.782 304.908L237.476 331.721C239.413 333.136 241.607 333.961 243.838 334.224C243.838 334.224 214.908 398.092 183.012 392.349C150.542 386.504 149.522 311.376 149.522 311.376C152.631 313.075 156.427 313.574 160.175 312.341Z\"/>";

static const std::string svgBlahajShadow =
"<path fill=\"#64A3E5\" fill-opacity=\"0.253286\" d=\"M292.741 90.656C288.394 90.5985 284.093 90.6695 279.899 90.8589C275.705 91.0487 271.603 91.3548 267.644 91.7313C259.735 92.4832 252.37 93.5562 245.895 94.6934C241.135 95.5301 237.479 96.3283 233.803 97.1483C224.969 99.1188 219.013 100.841 219.013 100.841C200.922 108.036 184.271 116.173 168.921 125.065C198.434 118.677 239.868 111.114 270.769 111.249C274.282 111.264 277.654 111.386 280.852 111.614C311.387 113.795 344.193 115.845 362.594 136.386C381.787 157.81 382.106 183.043 376.208 219.305C370.31 255.566 357.299 299.123 357.299 346.858C357.299 399.618 369.965 439.978 382.923 467.189C384.207 469.886 393.169 485.095 400.96 498.291C409.223 497.504 417.365 496.566 425.306 495.471C425.306 495.471 424.572 494.696 423.277 493.138C421.985 491.586 420.135 489.261 417.921 486.159C414.599 481.502 410.453 475.11 406.093 466.966C404.64 464.254 403.166 461.361 401.69 458.262C394.31 442.764 386.932 422.441 382.436 397.295C381.538 392.274 380.748 387.058 380.103 381.653V381.633C379.457 376.22 378.948 370.631 378.602 364.834C378.256 359.037 378.074 353.04 378.074 346.858C378.074 341.382 378.27 335.927 378.622 330.506C379.325 319.67 380.662 308.951 382.335 298.389V298.369C383.172 293.085 384.103 287.847 385.074 282.645C388.957 261.84 393.613 241.732 396.719 222.632C398.273 213.082 399.44 203.786 399.925 194.776C400.411 185.759 400.22 177.045 399.053 168.645C398.761 166.546 398.394 164.473 397.977 162.416C397.142 158.296 396.066 154.271 394.67 150.324C394.474 149.768 394.209 149.233 394.001 148.681C392.732 145.317 391.296 142.009 389.558 138.78C388.543 136.896 387.457 135.037 386.271 133.201C385.088 131.369 383.82 129.567 382.457 127.784C381.093 126 379.628 124.243 378.074 122.509C376.521 120.775 374.91 119.142 373.246 117.579C369.916 114.452 366.368 111.648 362.655 109.159C360.799 107.914 358.911 106.746 356.974 105.649C353.101 103.456 349.077 101.553 344.943 99.9075C342.871 99.0823 340.776 98.3341 338.654 97.6352C338.025 97.428 337.38 97.2637 336.747 97.0671C333.64 96.1024 330.492 95.2528 327.313 94.5311C326.789 94.4122 326.275 94.2582 325.75 94.1456H325.73C325.386 94.0718 325.04 94.0139 324.695 93.9427C318.483 92.66 312.181 91.782 305.908 91.2646C301.491 90.9003 297.088 90.7134 292.741 90.656Z\"/>";

static const std::string svgBlahajLinearGradients =
	"<defs>"
	"<linearGradient id=\"paint0_linear_1016_2\" x1=\"387.229\" y1=\"63.8651\" x2=\"499.912\" y2=\"487.896\" gradientUnits=\"userSpaceOnUse\">"
	"<stop stop-color=\"#81B6ED\"/>"
	"<stop offset=\"1\" stop-color=\"#1D75D2\"/>"
	"</linearGradient>"
	"<linearGradient id=\"paint1_linear_1016_2\" x1=\"227.141\" y1=\"105.442\" x2=\"233.882\" y2=\"482.493\" gradientUnits=\"userSpaceOnUse\">"
	"<stop stop-color=\"#FAFAFA\"/>"
	"<stop offset=\"1\" stop-color=\"#E6E6E6\"/>"
	"</linearGradient>"
	"<linearGradient id=\"paint2_linear_1016_2\" x1=\"196.68\" y1=\"302.312\" x2=\"187\" y2=\"393.5\" gradientUnits=\"userSpaceOnUse\">"
	"<stop stop-color=\"#D43074\"/>"
	"<stop offset=\"0.65\" stop-color=\"#D35B8D\"/>"
	"</linearGradient>"
	"</defs>";

#endif
