/*

*/

#include <stdlib.h>

#include "list.h"
#include "event.h"
#include "log.h" 	/*log.h must behind from event.h*/


static int epfd = -1 ;
static LIST_HEAD(ep_events_list);


static event_data_t *
ep_event_lookup(int fd)
{
	event_data_t *edt;

	list_for_each_entry(edt, ep_events_list, list) {
		if (edt->fd == fd)
			return edt;
	}
	return NULL;
}


void
ep_event_add(int epfd, int fd, event_handle_t handle, void *data)
{
	// dprintf("enter ep_event_add %d...\n",fd);

	struct epoll_event event;
	event_data_t *event_data;

	event_data = malloc(sizeof(event_data_t));
	event_data->handle = handle;
	event_data->fd = fd;

	event.events = EPOLLIN;
	event.data.ptr = event_data;
	
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		eprintf("epoll_ctl add failed");
		exit(EXIT_FAILURE);
	}

	list_add(&(event_data->list), &ep_events_list);
	return;
}


void
ep_event_del(int epfd, int fd)
{
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);

	event_data_t *edt = NULL;
	edt = (event_data_t*)(ep_event_lookup(fd));

	list_del(&(edt->list));
	return;
}


int 
ep_event_modify(int epfd, int fd, int events)
{
	struct epoll_event ev;
	struct event_data_t *edt;

	edt = ep_event_lookup(fd, ep_events_list);
	if (!edt) {
		eprintf("Cannot find event %d\n", fd);
		return -EINVAL;
	}

	memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = edt;

	return epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}


void 
event_add(int fd, event_handle_t handle, void *data)
{
    return ep_event_add(epfd, fd, handle, data);
}


void 
event_del(int fd)
{
    return ep_event_del(epfd, fd);
}


void 
event_modify(int fd, int events)
{
    return ep_event_modify(epfd, fd, events);
}


void 
process_events(void)
{
    dprintf("enter process events...\n");
    /*process the events list*/
    struct epoll_event events[MAX_EVENTS];
    int nevent  = -1;
    event_data_t *event_ptr; 

    for (;;) {
        if ((nevent = epoll_wait(epfd, events, MAX_EVENTS, -1)) == -1) {
            eprintf("epoll_wait failed");
            exit(EXIT_FAILURE);
        }

        int i = 0;
        for (;i < nevent;i++) {
            event_ptr = (event_data_t *)(events[i].data.ptr);
            event_ptr->handle(event_ptr->fd, events[i].events, event_ptr->data);
        }

    }
    return;
}


void 
event_init(void)
{
    INIT_LIST_HEAD(&ep_events_list);

    epfd = epoll_create(MAX_EVENTS);
    if (epfd == -1)
    {
        eprintf("epoll_create");
        exit(EXIT_FAILURE);
    }

    return;
}


