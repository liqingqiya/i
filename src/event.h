/*

*/
#ifndef _EVENT_H
#define _EVENT_H

#include <stdio.h>
#include <sys/epoll.h>

#include "list.h"

#define MAX_EVENTS 1024
//create epoll
static int ep = -1 ;
//create event list
LIST_HEAD_INIT(event_list);

static void epoll_event_init(void);
static void epoll_event_add(int fd, int event);
static void epoll_event_del();
static void epoll_process_event();

static void 
epoll_event_init()
{
	ep = epoll_create(MAX_EVENTS);
	if (ep == -1)
	{
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}
	return;
}

static void 
epoll_event_add(int epfd, int op, int fd, struct epoll_event *event)
{
	return;
}

static void
epoll_event_del()
{
	return;
}

static void
epoll_process_event()
{
	return;
}

#endif