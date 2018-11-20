CXX=g++
CC=gcc
CFLAGS=-Wall
INCLUDE=include/

all: server_sel client

server_sel: server_sel.o
server_sel.o: server_sel.cpp
	$(CXX) $(CFLAGS) -c server_sel.cpp -o server_sel.o

client: client.o comm_conn.o tcp_client.o
client.o: client.c comm_conn.o tcp_client.o
	$(CC) $(CFLAGS) -c client.c -o client.o -I $(INCLUDE)

comm_conn.o: $(INCLUDE)comm_conn.c $(INCLUDE)comm_conn.h
	$(CC) $(CFLAGS) -c $(INCLUDE)comm_conn.c -I $(INCLUDE)

tcp_client.o: $(INCLUDE)tcp_client.c $(INCLUDE)comm_conn.h
	$(CC) $(CFLAGS) -c $(INCLUDE)tcp_client.c -I $(INCLUDE)

run_serv_sel: server_sel
	./server_sel

run_client_sel: client
	./client
clean:
	rm server_sel *.o
	rm client *.o
