CFLAGS=-I../kdutils
LFLAGS=-lpthread -lpulse -lpulse-simple
OBJS=main.o ../kdutils/UdpSocket.o ../kdutils/kdutils.o pid.o

.cpp.o:
	g++ -c $(CFLAGS) $< -o $@ -std=c++11
.c.o:
	gcc -c $(CFLAGS) $< -o $@

server: $(OBJS)
	g++ $(CFLAGS) $(LFLAGS) $^ -o $@
