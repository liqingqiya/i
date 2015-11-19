server: server.o event.o
	@gcc -g -o server server.o event.o

server.o: server.c event.h
	@gcc -g -c server.c

event.o: event.h event.c
	@gcc -g -c event.c

client:
	@gcc -g -o client client.c

clean:
	@rm -fr *.o client server
