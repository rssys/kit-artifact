#define _GNU_SOURCE
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/resource.h>

#include "comm.h"

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                            } while (0)


int main() {
    if(init_exec_sync()) {
        perror("exec sync");
    }
    // Setup stage, similar to Syzkaller executor's 
    // do_sandbox_namespace().
    if (unshare(CLONE_NEWPID)) {
            perror("unshare");
    }
    int pid;
    pid = fork();
    if (pid == -1) {
            perror("fork");
    } else if (pid != 0) {
        int status;
        wait(&status);
        while(1) {
                sleep(100000);
        }
    } else {
        // Handshake
        if (exec_wait_nosig(10000)) {
                perror("exec wait");
        }
        if (exec_notify()) {
                perror("exec notify");
        }
        // Wait
        if (exec_wait_nosig(10000)) {
                perror("exec wait");
        }
        char buf[4096] = {0};
        buf[0] = '0';
        int p = getpriority(PRIO_USER, 0);
        snprintf(buf, sizeof(buf), "%d", p);
        if (exec_notify()) {
                perror("exec notify");
        }
        if (write(kReplyFd, buf, strlen(buf)) == -1) {
                perror("write");
        }
        return 0;
    }

    return 0;
}