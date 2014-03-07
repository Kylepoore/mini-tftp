#
# Kyle Poore
# March 3, 2014

CC = gcc -O3
LOCALHOST = localhost
REMOTEHOST = ec2-50-16-148-32.compute-1.amazonaws.com
FILENAME = test.txt

all: tftp

tftp: tftp.o server.o client.o fsm_server.o fsm_client_r.o fsm_client_w.o packet.o
	${CC} tftp.o server.o client.o fsm_server.o fsm_client_r.o fsm_client_w.o packet.o -o tftp

tftp.o: tftp.h tftp.c server.h client.h
	${CC} -c tftp.c

server.o: server.c server.h tftp.h fsm.h packet.h
	${CC} -c server.c

client.o: client.c client.h tftp.h fsm.h packet.h
	${CC} -c client.c

fsm_server.o: fsm.h tftp.h
	${CC} -c fsm_server.c

fsm_client_r.o: fsm.h tftp.h
	${CC} -c fsm_client_r.c

fsm_client_w.o: fsm.h tftp.h
	${CC} -c fsm_client_w.c

packet.o: tftp.h
	${CC} -c packet.c 

test:
	@echo Test directives have not been developed yet.
	@echo to test the server, run: make test-server
	@echo to test the client in write mode, run: make test-client-write
	@echo to test the client in read mode, run: make test-client-read

test-server:
	./tftp -lv

test-client-write:
	./tftp -wv ${LOCALHOST} ${FILENAME}

test-client-read:
	./tftp -rv ${LOCALHOST} ${FILENAME}

tftp-s:
	./tftp -l

tftp-c-r:
	.tftp -r ${REMOTEHOST} ${FILENAME}

tftp-c-w:
	.tftp -w ${REMOTEHOST} ${FILENAME}

submit:
	svn commit -m "submitted for grade"

clean:
	-rm *.o tftp
