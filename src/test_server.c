/*
server
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>


int 
main(int argc, char **argv)
{
	printf("begin server...\n");

	char ch;
	int srv_fd;
	int cli_fd;
	struct sockaddr_un srv_addr;
	struct sockaddr_un cli_addr;
	
	unlink("server_socket");
	srv_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	srv_addr.sun_family = AF_LOCAL;
	strcpy(srv_addr.sun_path, "server_socket");
	bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

	listen(srv_fd, 128);
	for(;;) {
		printf("server waiting \n");
		
		int cli_len = sizeof(cli_addr);
		cli_fd = accept(srv_fd, (struct sockaddr *)&cli_addr, &cli_len);
		read(cli_fd, &ch, 1);
		printf("after read: %c\n", ch);
		ch++;
		printf("before write: %c\n", ch);
		write(cli_fd, &ch, 1);
		close(cli_fd);
	}
	return 0;
}
