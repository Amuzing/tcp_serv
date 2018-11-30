#!/usr/bin/env python
# coding: utf-8

import socket
from random import choices
from string import ascii_uppercase, digits
from time import sleep


MSG_SIZE = 65535
RAND_STR_LEN = 10
CMD_NAME = b'0'
CMD_ADD  = b'1'
CMD_REM  = b'2'
CMD_LIST = b'3'

def get_rand_str(n):
    return ''.join(choices(ascii_uppercase + digits, k = n))

def get_conn_sock(_ip, _port):
    temp_fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    temp_fd.connect((_ip, _port))
    return temp_fd

def get_data_from_server(_socket):
    _socket.send(CMD_LIST)
    return str(_socket.recv(MSG_SIZE)).replace(r"\n", "\n").split("\n")


ip_address = "127.0.0.1"
ip_port = 5678
serv_name = "tcp_server_test"

try:
    s = get_conn_sock(ip_address, ip_port)



    s.recv(MSG_SIZE)

    s.send(CMD_NAME + bytes(serv_name, "utf-8"))

    my_test_strings = []
    for i in range(12):
        my_test_strings.append(str(get_rand_str(RAND_STR_LEN)))


    for item in my_test_strings:
        s.send(CMD_ADD + bytes(item, "utf-8"))
        sleep(0.125)


    data = get_data_from_server(s)

    for item in my_test_strings:
        temp = item + ", by " + str(serv_name)
        assert(temp in data), temp + " is not in data"
    else:
        print("Everything is ok, all strings arrived")


    for item in my_test_strings:
        s.send(CMD_REM + bytes(item, "utf-8"))
        sleep(0.125)


    data = get_data_from_server(s)

    for item in my_test_strings:
        temp = item + ", by " + str(serv_name)
        assert(temp not in data), temp + " is in data" 
    else:
        print("Everything is ok, all strings were deleted")

finally:
    s.close()
