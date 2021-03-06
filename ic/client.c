/*
client
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

#include "log.h"

int
main(int argc, char **argv)
{
	int sockfd;
	int len;
	struct sockaddr_un address;
	int result;
	char ch = 'B';
	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);

	address.sun_family = AF_LOCAL;
	strcpy(address.sun_path, "server_socket");

	len = sizeof(address);

	result = connect(sockfd, (struct sockaddr *)&address, len);
	if (result == -1)
	{
		eprintf("oops:client");
		exit(1);
	}

	dprintf("write to server = %c\n", ch);	
	write(sockfd, &ch, 1);

	read(sockfd, &ch, 1);
	dprintf("read from server = %c\n", ch);
	
	close(sockfd);
	exit(0);

}
