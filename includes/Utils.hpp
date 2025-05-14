/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 22:16:39 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/15 01:32:00 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <signal.h>
# include <iostream>
# include <unistd.h>
# include <cstring>
# include <cerrno>

extern volatile sig_atomic_t g_stop; // Global variable to indicate when to stop the server
extern int g_pipe_fds[2]; // Pipe file descriptors for signal handling

void signalHandler(int signum);
void setupSignalHandler();

std::string	trim(const std::string &s);
#endif
