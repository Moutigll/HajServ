#ifndef HAJ_LOG_H
#define HAJ_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/time.h>
#endif

#include "colors.h"

#ifndef HAJ_LOG_ENABLED
#define HAJ_LOG_ENABLED 1
#endif

#ifndef HAJ_LOG_LEVEL
#define HAJ_LOG_LEVEL LOG_DEBUG
#endif

typedef enum e_logLevel
{
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR
} e_logLevel;

#if HAJ_LOG_ENABLED

static inline void logGetTimestamp(char *buf, size_t len)
{
#if defined(_WIN32) || defined(_WIN64)
	SYSTEMTIME st;
	GetLocalTime(&st);
	snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
			 st.wYear, st.wMonth, st.wDay,
			 st.wHour, st.wMinute, st.wSecond,
			 st.wMilliseconds);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);

	struct tm tm;
	localtime_r(&tv.tv_sec, &tm);

	snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
			 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			 tm.tm_hour, tm.tm_min, tm.tm_sec,
			 tv.tv_usec / 1000);
#endif
}

static inline void logMessage(e_logLevel level, const char *file, int line, const char *fmt, ...)
{
	if (level < HAJ_LOG_LEVEL)
		return;

	const char *color = "";
	const char *prefix = "";

	switch (level)
	{
	case LOG_DEBUG: color = CYAN;   prefix = "DEBUG"; break;
	case LOG_INFO:  color = GREEN;  prefix = "INFO";  break;
	case LOG_WARN:  color = YELLOW; prefix = "WARN";  break;
	case LOG_ERROR: color = RED;    prefix = "ERROR"; break;
	default: prefix = "LOG"; break;
	}

	char ts[32];
	logGetTimestamp(ts, sizeof(ts));

	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "%s %s[%s]%s %s line %d:\n\t", ts, color, prefix, RESET, file, line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

#define LOG(level, fmt, ...) logMessage(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#else

#define LOG(level, fmt, ...) ((void)0)

#endif /* HAJ_LOG_ENABLED */

#endif /* HAJ_LOG_H */