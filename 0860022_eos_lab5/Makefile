CC=gcc
CFLAGS=-Wall

all:
	${CC} ${CFLAGS} -c lab5.c
	${CC} ${CFLAGS} -c socket_utils.c
	${CC} ${CFLAGS} lab5.o socket_utils.o -o lab5

clean: 
	${RM} lab5 lab5.o socket_utils.o
