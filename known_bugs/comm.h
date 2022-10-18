#ifndef __EXEC_COMM_H
#define __EXEC_COMM_H


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

const int kReplyFdOrignal = 3;
const int kWaitFdOrignal = 4;
const int kNotifyFdOrignal = 5;

const int kReplyFd = 237;
const int kWaitFd = 235;
const int kNotifyFd = 236;

static int flag_skip_prog = 0;

static int init_exec_sync() {
	if (dup2(kReplyFdOrignal, kReplyFd) == -1) {
		return -1;
	}
	close(kReplyFdOrignal);
	if (dup2(kWaitFdOrignal, kWaitFd) == -1) {
		return -1;
	}
	close(kWaitFdOrignal);
	if (dup2(kNotifyFdOrignal, kNotifyFd) == -1) {
		return -1;
	}
	close(kNotifyFdOrignal);
	if (getenv("SKIP_PROG") != NULL) {
		flag_skip_prog = 1;
	}
	return 0;
}

static int exec_wait(struct timeval timeout) {
	fd_set set;
	int ret;
	char c;


	FD_ZERO(&set);
	FD_SET(kWaitFd, &set);

	ret = select(kWaitFd + 1, &set, NULL, NULL, &timeout);
	if (ret == -1 || ret == 0) {
		goto out;
	}
	ret = read(kWaitFd, &c, 1);
	if (ret != 1) {
		ret = -1;
	} else {
		ret = 0;
	}
out:
	return ret;
}


static int exec_wait_nosig(uint64_t ms) {
	struct timespec start;
	uint64_t start_ms = 0;
	int ret = 0;
	while (1) {
		struct timeval timeout;
		if (clock_gettime(CLOCK_MONOTONIC, &start)) {
			return -1;
		}
		start_ms = (uint64_t)start.tv_sec * 1000 + (uint64_t)start.tv_nsec / 1000000;
		timeout = (struct timeval){
			.tv_sec = (long)(ms/1000),
			.tv_usec = (long)(1000*(ms%1000))
		};
		ret = exec_wait(timeout);
		if (ret == -1 && errno == EINTR) {
			struct timespec now;
			uint64_t now_ms;
			if (clock_gettime(CLOCK_MONOTONIC, &now)) {
				return -1;
			}
			now_ms = (uint64_t)now.tv_sec * 1000 + (uint64_t)now.tv_nsec / 1000000;
			ms -= now_ms - start_ms;
			continue;
		}
		break;
	}
	return ret;
}

static int exec_notify() {
	char c = 'y';
	if (write(kNotifyFd, &c, 1) != 1) {
		return -1;
	}
	return 0;
}


#endif