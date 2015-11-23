/*

*/

#ifndef _I_EVENT_H
#define _I_EVENT_H

#include <stdio.h>
#include <sys/epoll.h>

#include "list.h"


#define MAX_EVENTS 1024

typedef void (*event_handle_t)(int fd, void *data);

typedef struct {
	struct list_head		list;
	int 				fd;
	void 				*data;
	event_handle_t 			handle;
} event_data_t;

void event_init(void);
void event_add(int fd, event_handle_t handle, void *data);
void event_del(int fd);
void process_event(void);

#endif
