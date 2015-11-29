

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/sysmacros.h>

#include "log.h"
#include "util.h"

int set_non_blocking(int fd)
{
    int err;

    err = fcntl(fd, F_GETFL);
    if (err < 0) {
        eprintf("unable to get fd flags, %m\n");
    } else {
        err = fcntl(fd, F_SETFL, err | O_NONBLOCK);
        if (err == -1)
            eprintf("unable to set fd flags, %m\n");
        else
            err = 0;
    }
    return err;
}