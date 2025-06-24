/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Siege.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/24 22:15:00 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/24 23:09:57 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <curl/curl.h>

#define DEFAULT_THREADS 100
#define DEFAULT_DURATION 15
#define DEFAULT_URL "http://localhost:8080/"
#define OUTPUT_FILE "results.csv"

typedef struct s_mem
{
	char	*buffer;
	size_t	size;
}			t_mem;


typedef struct s_thread_arg
{
	FILE			*file;
	pthread_mutex_t	*mutex;
	struct timespec	end_time;
	char			*url;
}	t_thread_arg;

static size_t	discard_output(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	(void)ptr;
	(void)userdata;
	return (size * nmemb);
}

static long long	get_current_us(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL);
}

static long long	get_end_us(struct timespec end)
{
	return ((long long)end.tv_sec * 1000000LL + end.tv_nsec / 1000LL);
}

static size_t	write_to_memory(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	t_mem	*mem;
	size_t	realsize;

	mem = (t_mem *)userdata;
	realsize = size * nmemb;
	mem->buffer = realloc(mem->buffer, mem->size + realsize + 1);
	if (!mem->buffer)
		return (0);
	memcpy(mem->buffer + mem->size, ptr, realsize);
	mem->size += realsize;
	mem->buffer[mem->size] = '\0';
	return (realsize);
}

static void	*thread_fn(void *arg_void)
{
	t_thread_arg	*arg;
	long long		start_us;
	long long		end_us;
	long long		elapsed_us;
	long			status;
	CURL			*curl;
	CURLcode		res;
	t_mem			mem;

	arg = (t_thread_arg *)arg_void;
	end_us = get_end_us(arg->end_time);
	while (get_current_us() < end_us)
	{
		mem.buffer = malloc(1);
		mem.size   = 0;

		curl = curl_easy_init();
		if (!curl)
		{
			free(mem.buffer);
			return (NULL);
		}
		curl_easy_setopt(curl, CURLOPT_URL, arg->url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

		start_us = get_current_us();
		res = curl_easy_perform(curl);
		if (res == CURLE_OK)
			status = 200;
		else
			status = 0;
		elapsed_us = get_current_us() - start_us;

		pthread_mutex_lock(arg->mutex);
		fprintf(arg->file, "%lld,%lld,%ld,%zu\n",
				start_us, elapsed_us, status, mem.size);
		pthread_mutex_unlock(arg->mutex);

		curl_easy_cleanup(curl);
		free(mem.buffer);
	}
	return (NULL);
}


static int	parse_int(char *str)
{
	int	result;
	int	i;

	result = 0;
	i = 0;
	if (!str)
		return (-1);
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	if (str[i] != '\0')
		return (-1);
	return (result);
}

static char	*parse_arg_value(char *arg)
{
	char	*equal_pos;

	equal_pos = strchr(arg, '=');
	if (!equal_pos)
		return (NULL);
	return (equal_pos + 1);
}

static void	parse_args(int argc, char **argv,
		char **url, int *threads, int *duration)
{
	int		i;
	char	*val;

	*url = DEFAULT_URL;
	*threads = DEFAULT_THREADS;
	*duration = DEFAULT_DURATION;
	i = 1;
	while (i < argc)
	{
		if (strncmp(argv[i], "--url=", 6) == 0)
		{
			val = parse_arg_value(argv[i]);
			if (val && *val != '\0')
				*url = val;
		}
		else if (strncmp(argv[i], "-t=", 3) == 0)
		{
			val = parse_arg_value(argv[i]);
			if (val)
			{
				int tmp = parse_int(val);
				if (tmp > 0)
					*threads = tmp;
			}
		}
		else if (strncmp(argv[i], "-d=", 3) == 0)
		{
			val = parse_arg_value(argv[i]);
			if (val)
			{
				int tmp = parse_int(val);
				if (tmp > 0)
					*duration = tmp;
			}
		}
		i++;
	}
}

int	main(int argc, char **argv)
{
	pthread_t		*threads;
	t_thread_arg	arg;
	pthread_mutex_t	write_mutex;
	int				i;
	struct timespec	current_time;

	char			*url;
	int				threads_nb;
	int				duration_sec;

	parse_args(argc, argv, &url, &threads_nb, &duration_sec);
	curl_global_init(CURL_GLOBAL_DEFAULT);
	pthread_mutex_init(&write_mutex, NULL);
	arg.mutex = &write_mutex;
	arg.url = url;
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	arg.end_time.tv_sec = current_time.tv_sec + duration_sec;
	arg.end_time.tv_nsec = current_time.tv_nsec;
	arg.file = fopen(OUTPUT_FILE, "w");
	if (!arg.file)
		return (EXIT_FAILURE);
	fprintf(arg.file, "start_us,elapsed_us,http_code\n");
	threads = malloc(sizeof(pthread_t) * threads_nb);
	if (!threads)
	{
		fclose(arg.file);
		return (EXIT_FAILURE);
	}
	i = 0;
	while (i < threads_nb)
	{
		pthread_create(&threads[i], NULL, thread_fn, &arg);
		i++;
	}
	i = 0;
	while (i < threads_nb)
	{
		pthread_join(threads[i], NULL);
		i++;
	}
	free(threads);
	pthread_mutex_destroy(&write_mutex);
	fclose(arg.file);
	curl_global_cleanup();
	return (EXIT_SUCCESS);
}
