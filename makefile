CC=g++
CFLAGS=-Wall

all: server_sel

server_sel: server_sel.o
server_sel.o: server_sel.cpp
	$(CC) $(CFLAGS) -c server_sel.cpp -o server_sel.o

run: server_sel
	./server_sel

clean:
	rm server_sel *.o