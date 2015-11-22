/*
server
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#include "event.h"

void ipc_recv_send(int fd, void *data);
void ipc_accpet(int fd, void *data);
void server_init(void);

static void signal_catch(int signo)
{
}

void 
ipc_accpet(int fd, void *data)
{
	printf("enter ipc accpet handle ...\n");

	struct sockaddr_un cli_add;
	int cli_fd, cli_len, read_n;

	cli_len = sizeof(cli_add);
	cli_fd = accept(fd, (struct sockaddr *)&cli_add, &cli_len);
	
	event_add(cli_fd, ipc_recv_send, NULL);

	return ;
}

void 
ipc_recv_send(int fd, void *data)
{
	int read_n;
	char ch;

	read_n = read(fd, &ch, 1);
	if (read_n == -1) {
		perror("[ipc_recv_sed]: read failed");
		exit(EXIT_FAILURE);	
	} else if (read_n == 0) {
		close(fd);
		return;
	}
	printf("[ipc_recv_send] read from client: %c\n", ch);

	ch++;
	
	printf("[ipc_recv_send] write to client: %c\n", ch);
	write(fd, &ch, 1);
}

void 
server_init(void)
{
	int srv_fd;
	struct sockaddr_un srv_add;
	
	unlink("server_socket");
	srv_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	srv_add.sun_family = AF_LOCAL;
	strcpy(srv_add.sun_path, "server_socket");
	bind(srv_fd, (struct sockaddr *)&srv_add, sizeof(srv_add));
	listen(srv_fd, 128);
	
	event_add(srv_fd, ipc_accpet, NULL);
}

int 
main(int argc, char **argv)
{
	printf("begin server...\n");

	signal(SIGPIPE, SIG_IGN);
	// signal(SIGTERM, SIG_IGN)

	//init epoll event
	event_init();

	//init server
	server_init();
	
	//process events
	process_events();
	return 0;
}
