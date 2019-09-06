/*****************************************************************************
#                                                                            #
#    uStreamer - Lightweight and fast MJPG-HTTP streamer.                    #
#                                                                            #
#    Copyright (C) 2018  Maxim Devaev <mdevaev@gmail.com>                    #
#                                                                            #
#    This program is free software: you can redistribute it and/or modify    #
#    it under the terms of the GNU General Public License as published by    #
#    the Free Software Foundation, either version 3 of the License, or       #
#    (at your option) any later version.                                     #
#                                                                            #
#    This program is distributed in the hope that it will be useful,         #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of          #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
#    GNU General Public License for more details.                            #
#                                                                            #
#    You should have received a copy of the GNU General Public License       #
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.  #
#                                                                            #
*****************************************************************************/


#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include <pthread.h>

#include "tools.h"


enum {
	LOG_LEVEL_INFO,
	LOG_LEVEL_PERF,
	LOG_LEVEL_VERBOSE,
	LOG_LEVEL_DEBUG,
} log_level;

bool log_colored;

pthread_mutex_t log_mutex;


#define LOGGING_INIT { \
		log_level = LOG_LEVEL_INFO; \
		log_colored = isatty(1); \
		assert(!pthread_mutex_init(&log_mutex, NULL)); \
	}

#define LOGGING_DESTROY assert(!pthread_mutex_destroy(&log_mutex))

#define LOGGING_LOCK	assert(!pthread_mutex_lock(&log_mutex))
#define LOGGING_UNLOCK	assert(!pthread_mutex_unlock(&log_mutex))


#define COLOR_GRAY		"\x1b[30;1m"
#define COLOR_RED		"\x1b[31;1m"
#define COLOR_GREEN		"\x1b[32;1m"
#define COLOR_YELLOW	"\x1b[33;1m"
#define COLOR_BLUE		"\x1b[34;1m"
#define COLOR_CYAN		"\x1b[36;1m"
#define COLOR_RESET		"\x1b[0m"


#define SEP_INFO(_ch) { \
		LOGGING_LOCK; \
		for (int _i = 0; _i < 80; ++_i) { \
			putchar(_ch); \
		} \
		putchar('\n'); \
		fflush(stdout); \
		LOGGING_UNLOCK; \
	}

#define SEP_DEBUG(_ch) { \
		if (log_level >= LOG_LEVEL_DEBUG) { \
			SEP_INFO(_ch); \
		} \
	}


#define LOG_PRINTF_NOLOCK(_label_color, _label, _msg_color, _msg, ...) { \
		if (log_colored) { \
			printf(COLOR_GRAY "-- " _label_color _label COLOR_GRAY " [%.03Lf tid=%d]" " -- " COLOR_RESET _msg_color _msg COLOR_RESET, \
				get_now_monotonic(), get_thread_id(), ##__VA_ARGS__); \
		} else { \
			printf("-- " _label " [%.03Lf tid=%d] -- " _msg, \
				get_now_monotonic(), get_thread_id(), ##__VA_ARGS__); \
		} \
		putchar('\n'); \
		fflush(stdout); \
	}

#define LOG_PRINTF(_label_color, _label, _msg_color, _msg, ...) { \
		LOGGING_LOCK; \
		LOG_PRINTF_NOLOCK(_label_color, _label, _msg_color, _msg, ##__VA_ARGS__); \
		LOGGING_UNLOCK; \
	}

#define LOG_ERROR(_msg, ...) { \
		LOG_PRINTF(COLOR_RED, "ERROR", COLOR_RED, _msg, ##__VA_ARGS__); \
	}

#define LOG_PERROR(_msg, ...) { \
		char _buf[1024] = ""; \
		char *_ptr = errno_to_string(_buf, 1024); \
		LOG_ERROR(_msg ": %s", ##__VA_ARGS__, _ptr); \
	}

#define LOG_INFO(_msg, ...) { \
		LOG_PRINTF(COLOR_GREEN, "INFO ", "", _msg, ##__VA_ARGS__); \
	}

#define LOG_INFO_NOLOCK(_msg, ...) { \
		LOG_PRINTF_NOLOCK(COLOR_GREEN, "INFO ", "", _msg, ##__VA_ARGS__); \
	}

#define LOG_PERF(_msg, ...) { \
		if (log_level >= LOG_LEVEL_PERF) { \
			LOG_PRINTF(COLOR_CYAN, "PERF ", COLOR_CYAN, _msg, ##__VA_ARGS__); \
		} \
	}

#define LOG_PERF_FPS(_msg, ...) { \
		if (log_level >= LOG_LEVEL_PERF) { \
			LOG_PRINTF(COLOR_YELLOW, "PERF ", COLOR_YELLOW, _msg, ##__VA_ARGS__); \
		} \
	}

#define LOG_VERBOSE(_msg, ...) { \
		if (log_level >= LOG_LEVEL_VERBOSE) { \
			LOG_PRINTF(COLOR_BLUE, "VERB ", COLOR_BLUE, _msg, ##__VA_ARGS__); \
		} \
	}

#define LOG_DEBUG(_msg, ...) { \
		if (log_level >= LOG_LEVEL_DEBUG) { \
			LOG_PRINTF(COLOR_GRAY, "DEBUG", COLOR_GRAY, _msg, ##__VA_ARGS__); \
		} \
	}


INLINE char *errno_to_string(char *buf, size_t size) {
#	if defined(__GLIBC__) && defined(_GNU_SOURCE)
	return strerror_r(errno, buf, size);
#	else
	strerror_r(errno, buf, size);
	return buf;
#	endif
}
