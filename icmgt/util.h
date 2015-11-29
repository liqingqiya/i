#ifndef __UTIL_H__
#define __UTIL_H__

#include <byteswap.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/falloc.h>
#include <signal.h>
#include <syscall.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <linux/types.h>


#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#define DIV_ROUND_UP(x, y) (((x) + (y) - 1) / (y))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))

#define DEFDMODE    (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#define DEFFMODE    (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

extern int set_non_blocking(int fd);


#define zalloc(size)            \
({                  \
    void *ptr = malloc(size);   \
    if (ptr)            \
        memset(ptr, 0, size);   \
    else                \
        eprintf("%m\n");    \
    ptr;                \
})

#define min(x,y) ({ \
    typeof(x) _x = (x); \
    typeof(y) _y = (y); \
    (void) (&_x == &_y);        \
    _x < _y ? _x : _y; })

#define max(x,y) ({ \
    typeof(x) _x = (x); \
    typeof(y) _y = (y); \
    (void) (&_x == &_y);        \
    _x > _y ? _x : _y; })

#endif