CFLAGS=
LFLAGS=-lpthread
OBJS=main.o

.cpp.o:
	g++ -c $(CFLAGS) $< -o $@
.c.o:
	gcc -c $(CFLAGS) $< -o $@

out: $(OBJS)
	g++ $(CFLAGS) $(LFLAGS) $^ -o $@
