server: server.o event.o log.o
	@gcc -g -o server server.o event.o log.o

server.o: server.c event.h log.h
	@gcc -g -c server.c

event.o: event.c event.h log.h
	@gcc -g -c event.c

log.o: log.c log.h
	@gcc -g -c log.c

client:
	@gcc -g -o client client.c

clean:
	@rm -fr *.o client server
