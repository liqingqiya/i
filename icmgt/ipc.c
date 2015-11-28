/*
该文件负责接受终端用户的请求逻辑
终端客户发过来的是json字符串
该文件负责监听，解析，并执行命令
*/

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include "log.h"            /*log*/
#include "event.h"          /*event*/
#include "mgmt.h"           /*tgt handle*/
#include "ipc.h"


#define ICMGT_IPC_DIR     "/var/run/icmgt"
#define ICMGT_IPC_NAMESPACE   ICMGT_IPC_DIR"/socket"

static int ipc_fd;
char icmgt_path[256];

static struct ipc_task *ipc_task_alloc(void);
static void ipc_task_free(struct ipc_task *ipc_task);

/**/
enum ipc_task_state {
    MTASK_STATE_HDR_RECV,
    MTASK_STATE_PDU_RECV,
    MTASK_STATE_HDR_SEND,
    MTASK_STATE_PDU_SEND,
};

/**/
struct ipc_task {
    enum ipc_task_state ipc_state;
    int retry;
    int done;
    struct tgtadm_req req;
    char *req_buf;
    int req_bsize;
    struct tgtadm_rsp rsp;
    struct concat_buf rsp_concat;
};


static struct ipc_task *
ipc_task_alloc(void)
{

}

static void
ipc_task_free(struct ipc_task *task)
{

}

static int 
ipc_accept(int accept_fd)
{
    struct sockaddr addr;
    socklen_t len;
    int fd;

    len = sizeof(addr);
    fd = accept(accept_fd, (struct sockaddr *) &addr, &len);
    if (fd < 0)
        eprintf("can't accept a new connection, %m\n");
    return fd;
}


static void 
ipc_accept_handler(int accept_fd, int events, void *data)
{
    int fd, err;
    struct ipc_task *ipc_task;

    fd = ipc_accept(accept_fd);
    if (fd < 0) {
        eprintf("failed to accept a socket\n");
        return;
    }

    err = set_non_blocking(fd);
    if (err) {
        eprintf("failed to set a socket non-blocking\n");
        goto out;
    }

    ipc_task = ipc_task_alloc();
    if (!ipc_task)
        goto out;

    err = events_add(fd, EPOLLIN, ipc_recv_send_handler, ipc_task);
    if (err) {
        eprintf("failed to add a socket to epoll %d\n", fd);
        ipc_task_free(ipc_task);
        goto out;
    }

    return;
out:
    if (fd > 0)
        close(fd);
    return;
}

static void 
ipc_recv_send_handler(int accept_fd, int events, void *data)
{
    
}

/*
创建与终端客户通信的套接字
*/
int 
ipc_init(void)
{
    int fd = 0, err;
    struct sockaddr_un addr;
    struct stat st = {0};
    char *path;

    /*create socket dir*/
    if ((path = getenv("ICMGT_IPC_SOCKET")) == NULL) {
        path = ICMGT_IPC_NAMESPACE;
        if (stat(ICMGT_IPC_DIR, &st) == -1)
            mkdir(ICMGT_IPC_DIR, 0755);
    }

    /*create the ipc socket*/
    fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0) {
        eprintf("can't open a socket, %m\n");
        goto close_fd;
    }

    snprintf(icmgt_path, sizeof(icmgt_path), "%s", path);
    unlink(icmgt_path);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, icmgt_path, sizeof(addr.sun_path));
    /*bind*/
    err = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
    if (err) {
        eprintf("can't bind a socket, %m\n");
        goto close_ipc_fd;
    }
    /*listen*/
    err = listen(fd, 32);
    if (err) {
        eprintf("can't listen a socket, %m\n");
        goto close_ipc_fd;
    }

    err = events_add(fd, EPOLLIN, ipc_accept_handler, NULL);
    if (err)
        goto close_ipc_fd;

    ipc_fd = fd;

    return 0;

close_ipc_fd:
    close(fd);
    return -1;
}
