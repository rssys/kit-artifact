#define _GNU_SOURCE
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdarg.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/icmp.h>

#include "comm.h"

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                            } while (0)
int main() {
    char x;
    
        if(init_exec_sync()) {
            perror("exec sync");
            exit(-1);
        }
        // Setup stage, similar to Syzkaller executor's 
        // do_sandbox_namespace().
        // Bind the socket.
        if (unshare(CLONE_NEWNET)) {
                perror("unshare");
                exit(-1);
        }
        int nl_socket;
        nl_socket = socket(AF_NETLINK, (SOCK_DGRAM | SOCK_CLOEXEC), NETLINK_KOBJECT_UEVENT);
        if (nl_socket < 0) {
            errExit("create netlink socket");
        }
        struct sockaddr_nl src_addr;
        // Prepare source address
        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = getpid();
        src_addr.nl_groups = -1;
        int ret;
        ret = bind(nl_socket, (struct sockaddr*) &src_addr, sizeof(src_addr));
        if (ret < 0) {
            errExit("bind netlink");
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

        if (exec_wait_nosig(10000)) {
                perror("exec wait");
                exit(-1);
        }

        char msg[4096] = {0};

        char *trace = "";
        int st = 0;
        while(1) {
                int r = recv(nl_socket, msg+st, sizeof(msg)-st, MSG_DONTWAIT);
                if (r < 0 || r == 0) {
                        break;
                }
                int l = strlen(msg);
                msg[l] = ';';
                st = l+1;
                msg[l+1] = '\0';
        }
        trace = msg;
        if (!strlen(trace)) {
                trace = "recv()=0";
        }
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