/*
* 该文件负责接受终端用户的请求逻辑
* 终端客户发过来的是json字符串
* 该文件负责监听，解析，并执行命令
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

struct ipc_task *ipc_task_alloc(void);
void ipc_task_free(struct ipc_task *ipc_task);

/**/
enum ipc_task_state {
    IPC_TASK_STATE_SIZE_RECV,       /*size 4 bytes*/
    IPC_MTASK_STATE_STR_RECV,       /*string*/
    // IPC_MTASK_STATE_SIZE_SEND,      /*size*/
    IPC_MTASK_STATE_STR_SEND,       /*string*/
};

/**/
struct ipc_task {
    enum ipc_task_state ipc_state;
    int retry;
    int done;                            /**/
    struct tgtadm_req req;              /**/
    char *req_buf;                       /**/
    int req_bsize;                      /**/
    struct tgtadm_rsp rsp;              /**/
    struct concat_buf rsp_concat;       /**/
};


/*allocate a ipc task struct*/
static struct ipc_task *
ipc_task_alloc(void)
{
    struct ipc_task *itask;
    itask = zalloc(sizeof(*itask));
    if (!itask) {
        eprintf("can't allocate itask\n");
        return NULL;
    }

    itask->itask_state = IPC_TASK_STATE_SIZE_RECV;
    dprintf("itask:%p\n", itask);
    return itask;
}


static void
ipc_task_free(struct ipc_task *itask)
{
    dprintf("itask:%p\n", itask);
    if (itask->req_buf) {
        /*req_buf*/
        free(itask->req_buf);
    }
    /*rsp_concat*/
    concat_buf_release(&itask->rsp_concat);
    free(itask);
}


int 
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


void 
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

void 
ipc_recv_send_handler(int accept_fd, int events, void *data)
{
    int err, len;
    char *p;
    struct mgmt_task *mtask = data;
    struct tgtadm_req *req = &mtask->req;
    struct tgtadm_rsp *rsp = &mtask->rsp;

    switch (mtask->mtask_state) {
    case MTASK_STATE_HDR_RECV:
        len = sizeof(*req) - mtask->done;
        err = read(fd, (char *)req + mtask->done, len);
        if (err > 0) {
            mtask->done += err;
            if (mtask->done == sizeof(*req)) {
                mtask->req_bsize = req->len - sizeof(*req);
                if (!mtask->req_bsize) {
                    err = mtask_received(mtask, fd);
                    if (err)
                        goto out;
                } else {
                    /* the pdu exists */
                    if (mtask->req_bsize > MAX_MGT_BUFSIZE) {
                        eprintf("mtask buffer len: %d too large\n",
                            mtask->req_bsize);
                        mtask->req_bsize = 0;
                        goto out;
                    }
                    mtask->req_buf = zalloc(mtask->req_bsize);
                    if (!mtask->req_buf) {
                        eprintf("can't allocate mtask buffer len: %d\n",
                            mtask->req_bsize);
                        mtask->req_bsize = 0;
                        goto out;
                    }
                    mtask->mtask_state = MTASK_STATE_PDU_RECV;
                    mtask->done = 0;
                }
            }
        } else
            if (errno != EAGAIN)
                goto out;

        break;
    case MTASK_STATE_PDU_RECV:
        len = mtask->req_bsize - mtask->done;
        err = read(fd, mtask->req_buf + mtask->done, len);
        if (err > 0) {
            mtask->done += err;
            if (mtask->done == mtask->req_bsize) {
                err = mtask_received(mtask, fd);
                if (err)
                    goto out;
            }
        } else
            if (errno != EAGAIN)
                goto out;

        break;
    case MTASK_STATE_HDR_SEND:
        p = (char *)rsp + mtask->done;
        len = sizeof(*rsp) - mtask->done;

        err = write(fd, p, len);
        if (err > 0) {
            mtask->done += err;
            if (mtask->done == sizeof(*rsp)) {
                if (rsp->len == sizeof(*rsp))
                    goto out;
                mtask->done = 0;
                /*lq: write the rsq, then change the state*/
                mtask->mtask_state = MTASK_STATE_PDU_SEND;
            }
        } else
            if (errno != EAGAIN)
                goto out;

        break;
    case MTASK_STATE_PDU_SEND:
        err = concat_write(&mtask->rsp_concat, fd, mtask->done);
        if (err >= 0) {
            mtask->done += err;
            /*lq: done*/
            if (mtask->done == (rsp->len - sizeof(*rsp)))
                goto out;
        } else
            if (errno != EAGAIN)
                goto out;

        break;
    default:
        eprintf("unknown state %d\n", mtask->mtask_state);
    }

    return;
out:
    if (req->mode == MODE_SYSTEM && req->op == OP_DELETE && !rsp->err)
        system_active = 0;
    tgt_event_del(fd);
    close(fd);
    mtask_free(mtask);
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

void 
ipc_exit(void)
{
    event_del(ipc_fd);
    close(ipc_fd);
}