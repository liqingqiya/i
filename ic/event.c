/*
event drive
*/

#include <stdlib.h>

#include "list.h"
#include "event.h"
/*log.h must behind from event.h*/
#include "log.h"

//create epoll
static int epfd = -1 ;

//create event list
static struct list_head events_list;


static void *
find_event_by_fd(int fd)
{
	event_data_t *ed = NULL;
	/*for each list entry*/
	list_for_each_entry(ed, &events_list, list)
	{
		if (ed->fd == fd)
		{
			return ed;
		}
	}
	return NULL;
}


void 
event_init(void)
{
	INIT_LIST_HEAD(&events_list);

	epfd = epoll_create(MAX_EVENTS);
	if (epfd == -1)
	{
		eprintf("epoll_create");
		exit(EXIT_FAILURE);
	}
	return;
}

void 
event_add(int fd, event_handle_t handle, void *data)
{
	//debug
	dprintf("enter event_add %d...\n",fd);

	struct epoll_event event;
	event_data_t *event_data;

	/*create a event*/
	event_data = malloc(sizeof(event_data_t));
	event_data->handle = handle;
	event_data->fd = fd;

	event.events = EPOLLIN;
	event.data.ptr = event_data;
	
	/*add to epoll*/
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		eprintf("epoll_ctl add failed");
		exit(EXIT_FAILURE);
	}

	/*add to events list*/
	list_add(&(event_data->list), &events_list);
	
	return;
}

void
event_del(int fd)
{

	/*delete from epoll*/
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);

	/*delete from events list*/
	event_data_t *ed = NULL;
	ed = (event_data_t*)(find_event_by_fd(fd));
	list_del(&(ed->list));

	return;
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
		dprintf("before epoll_wait....\n");
		if ((nevent = epoll_wait(epfd, events, MAX_EVENTS, -1)) == -1) {
			eprintf("epoll_wait failed");
			exit(EXIT_FAILURE);
		}

		dprintf("after epoll_wait %d....\n", nevent);
		int i = 0;
		for (;i < nevent;i++) {
			event_ptr = (event_data_t *)(events[i].data.ptr);
			/*process handle*/
			event_ptr->handle(event_ptr->fd, event_ptr->data);
		}

	}
	return;
}
