server: server.o event.o
	@cc -o server server.o event.o

server.o: server.c event.h
	@cc -c server.c

event.o: event.h event.c
	@cc -c event.c

client:
	@cc -o client client.c

clean:
	@rm -fr *.o
