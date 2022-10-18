#define _GNU_SOURCE
#include "comm.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>

int main() {
        if(init_exec_sync()) {
                perror("exec sync");
                exit(-1);
        }
        // Setup stage, similar to Syzkaller executor's 
        // do_sandbox_namespace().
        if (unshare(CLONE_NEWNET)) {
                perror("unshare");
                exit(-1);
        }
        // Handshake
        if (exec_wait_nosig(10000)) {
                perror("exec wait");
                exit(-1);
        }
        if (exec_notify()) {
                perror("exec notify");
                exit(-1);
        }
        // Wait for the preceding sender program to execute
        if (exec_wait_nosig(10000)) {
                perror("exec wait");
                exit(-1);
        }
        if (exec_notify()) {
                perror("exec notify");
                exit(-1);
        }
        fprintf(stderr, "Victim: I start!\n");
        if (unshare(CLONE_NEWNET)) {
                perror("unshare");
                exit(-1);
        }
        system("ip a add dev lo 127.0.0.1/8");
        system("ipvsadm -A -t 10.0.0.0:6");
        system("ipvsadm -A -t 10.0.0.0:249");
        char *trace = "";
        int fd = open("/proc/net/ip_vs", O_RDONLY);
        if (fd == -1) {
                trace = "open()=-1";
                goto out;
        }
        char buf[4096] = {0};
        if (read(fd, buf, sizeof(buf)) == -1) {
                trace = "read()=-1";
                goto out;
        }
        trace = buf;
out:
        if (write(kReplyFd, trace, strlen(trace)) == -1) {
                perror("write");
                exit(-1);
        }
        if (exec_notify()) {
                perror("exec notify");
                exit(-1);
        }
        return 0;
}