import socket
import sys
from random import choices
from string import ascii_uppercase, digits
from time import sleep
import random

UINT32_T_SIZE = 4
MSG_COUNT = 1000
MSG_SIZE = 655355
MSG_MAX_SIZE = 100
CMD_NAME = b'0'
CMD_ADD  = b'1'
CMD_REM  = b'2'
CMD_LIST = b'3'

def serialize_int(x):
    x = socket.htonl(x)
    return x.to_bytes(UINT32_T_SIZE, sys.byteorder)

def deserialize_int(l):
    x = int.from_bytes(l, sys.byteorder)
    return socket.ntohl(x)

def send_str(s, msg):
    total_size = UINT32_T_SIZE + len(msg)
    new_msg = serialize_int(total_size) + msg
    s.send(new_msg)
    return

def recv_str(s):
    temp_data = s.recv(MSG_SIZE)
    total_size = deserialize_int(temp_data[:4])
    print(total_size, len(temp_data), temp_data)
    assert(total_size == len(temp_data))
    return temp_data[4:]
    
def get_rand_str(n):
    return ''.join(choices(ascii_uppercase + digits, k = n))

def get_conn_sock(_ip, _port):
    temp_fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    temp_fd.connect((_ip, _port))
    return temp_fd

def get_data_from_server(_socket):
    send_str(_socket, CMD_LIST)
    # _socket.send(CMD_LIST)
    return str(recv_str(_socket)).replace(r"\n", "\n").split("\n")
    # return str(_socket.recv(MSG_SIZE)).replace(r"\n", "\n").split("\n")


ip_address = "127.0.0.1"
ip_port = 5678
serv_name = "tcp_server_test"

random.seed()

try:
    s = get_conn_sock(ip_address, ip_port)


    recv_str(s)
    # s.recv(MSG_SIZE)

    send_str(s, CMD_NAME + bytes(serv_name, "utf-8"))
    # s.send(CMD_NAME + bytes(serv_name, "utf-8"))

    my_test_strings = []
    for i in range(MSG_COUNT):
        my_test_strings.append(str(get_rand_str(random.randint(1, MSG_MAX_SIZE))))


    for item in my_test_strings:
        send_str(s, CMD_ADD + bytes(item, "utf-8"))
        # s.send(CMD_ADD + bytes(item, "utf-8"))
        # sleep(0.125)


    data = get_data_from_server(s)

    for item in my_test_strings:
        temp = item + ", by " + str(serv_name)
        assert(temp in data), temp + " is not in data"
    else:
        print("Everything is ok, all strings arrived")
        print(data)


    for item in my_test_strings:
        send_str(s, CMD_REM + bytes(item, "utf-8"))
        # s.send(CMD_REM + bytes(item, "utf-8"))
        sleep(0.125)


    data = get_data_from_server(s)
    

    for item in my_test_strings:
        temp = item + ", by " + str(serv_name)
        assert(temp not in data), temp + " is in data" 
    else:
        print("Everything is ok, all strings were deleted")

finally:
    s.close()

