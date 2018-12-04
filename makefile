CXX=g++
CC=gcc
CFLAGS=-Wall -g
CXXFLAGS=$(CFLAGS) -std=c++17
INCLUDE=include/

all: server_sel client

server_sel: server_sel.o tcp_comm_conn.o tcp_server.o
	$(CXX) $(CXXFLAGS) server_sel.o tcp_comm_conn.o tcp_server.o -o server_sel
server_sel.o: server_sel.cpp $(INCLUDE)server_sel.h
	$(CXX) $(CXXFLAGS) -c server_sel.cpp -o server_sel.o -I $(INCLUDE)

server_pol: server_pol.o tcp_comm_conn.o tcp_server.o
	$(CXX) $(CXXFLAGS) server_pol.o tcp_comm_conn.o tcp_server.o -o server_pol
server_pol.o: server_pol.cpp
	$(CXX) $(CXXFLAGS) -c server_pol.cpp -o server_pol.o -I $(INCLUDE)

server_epol: server_epol.o tcp_comm_conn.o tcp_server.o
	$(CXX) $(XXCFLAGS) server_epol.o tcp_comm_conn.o tcp_server.o -o server_epol
server_epol.o: server_pol.cpp
	$(CXX) $(CXXFLAGS) -c server_epol.cpp -o server_epol.o -I $(INCLUDE)

client: client.o tcp_comm_conn.o tcp_client.o
client.o: client.c tcp_comm_conn.o tcp_client.o
	$(CC) $(CFLAGS) -c client.c -o client.o -I $(INCLUDE)

tcp_comm_conn.o: $(INCLUDE)tcp_comm_conn.c $(INCLUDE)tcp_comm_conn.h
	$(CC) $(CFLAGS) -c $(INCLUDE)tcp_comm_conn.c -I $(INCLUDE)

tcp_client.o: $(INCLUDE)tcp_client.c $(INCLUDE)tcp_comm_conn.h
	$(CC) $(CFLAGS) -c $(INCLUDE)tcp_client.c -I $(INCLUDE)

tcp_server.o: $(INCLUDE)tcp_server.cpp $(INCLUDE)tcp_comm_conn.h $(INCLUDE)tcp_server.h
	$(CXX) $(CXXFLAGS) -c $(INCLUDE)tcp_server.cpp -I $(INCLUDE)

run_serv_sel: server_sel
	./server_sel

run_client_sel: client
	./client
clean:
	rm server_sel *.o
	rm client