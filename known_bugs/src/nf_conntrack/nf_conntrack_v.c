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
        if (unshare(CLONE_NEWNET)) {
                perror("unshare");
        }
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
        int fd = open("/proc/sys/net/netfilter/nf_conntrack_max", O_RDONLY);
        if (fd == -1) {
                perror("open");
        }
        char buf[4096] = {0};
        buf[0] = '0';
        if (read(fd, buf, sizeof(buf)) == -1) {
                perror("read");
        }
        if (exec_notify()) {
                perror("exec notify");
        }
        if (write(kReplyFd, buf, strlen(buf)) == -1) {
                perror("write");
        }
        while(1) {
                sleep(100000);
        }
        return 0;

}