CXX=g++
CFLAGS=-Wall -lpthread


all:
	${CXX} ${CFLAGS} -c hw2.cpp
	${CXX} ${CFLAGS} -c socket_utils.c
	${CXX} ${CFLAGS} -c parking_lot.cpp
	${CXX} hw2.o socket_utils.o parking_lot.o ${CFLAGS} ${INC_FLAGS} -o hw2

clean:
	${RM} hw2.o socket_utils.o parking_lot.o hw2