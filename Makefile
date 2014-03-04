#
# Kyle Poore
# March 3, 2014

CC = gcc
LOCALHOST = localhost
REMOTEHOST = ec2-50-19-171-60.compute-1.amazonaws.com

all: tftp

tftp: tftp.o server.o client.o
	${CC} tftp.o server.o client.o -o tftp

tftp.o: tftp.h tftp.c server.h client.h
	${CC} -c tftp.c

server.o: server.c server.h tftp.h
	${CC} -c server.c

client.o: client.c client.h tftp.h
	${CC} -c client.c

test:
	@echo For a local test, for run test-local-s in one window, then test-local-c in another.
	@echo For a netbounce, run nb-s in the far computer, then nb-c in the near computer.

test-local-c:
	./tftp-server

test-local-s:
	./tftp-client ${LOCALHOST} hello_world


nb-s:
	@echo Ready to bounce!
	./tftp-server

nb-c:
	.tftp-client ${REMOTEHOST} "The amazing net bounce!"

submit:
	svn commit -m "submitted for grade"

clean:
	-rm *.o tftp
