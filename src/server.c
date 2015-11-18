#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

#include "event.h"

void 
ipc_accpet_handle(int fd, void *data)
{
	char buf[128];
	struct sockaddr_un cli_add;
	int cli_fd, cli_len, read_n;

	cli_len = sizeof(cli_add);
	cli_fd = accept(fd, (struct sockaddr *)&cli_add, &cli_len);

	read_n = read(fd, buf, 128);
	printf("buf: %s", buf);

	return ;
}

void 
server_init(void)
{
	int srv_fd;
	struct sockaddr_un srv_add;
	
	unlink("server_socket");
	srv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	srv_add.sun_family = AF_UNIX;
	strcpy(srv_add.sun_path, "server_socket");
	bind(srv_fd, (struct sockaddr *)&srv_add, sizeof(srv_add));
	listen(srv_fd, 128);
	
	event_add(srv_fd, ipc_accpet_handle, NULL);
}

int 
main(int argc, char **argv)
{
	return 0;
}
