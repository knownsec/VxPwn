#!/usr/bin/env python
# coding=utf-8

# probe on udp port 17185, VxWorks WDBRPC V1 & V2
# By dog2@404

import socket
import struct


def scanV1(host, port=17185, timeout=5):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    payload_hex = 'cc6ff7e2000000000000000255555555000000010000000100000000000000000000000000000000ffff2e700000003026b00001'
    try:
        sock.sendto(payload_hex.decode('hex'), (host, port))
        banner = sock.recv(65536)
    except socket.error as err:
        return None, ''

    return 'vxworks' in banner.lower(), banner


def scanV2(host, port=17185, timeout=5):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    connReq = ''.join([
        struct.pack('>I', 0),  # msgid
        '\x00' * 4,  # msgcall
        '\x00\x00\x00\x02',  # rpc version
        '\x55' * 4,  # wdb programe number
        '\x00\x00\x00\x01',  # programe version
        struct.pack('>I', 122),  # function number: WDB_TARGET_CONNECT2 = 122
        '\x00' * 16,
        '\x00' * 4,
        '\x00\x00\x00\x30',  # packet length
        struct.pack('>I', 0),  # msg seq
    ])

    try:
        sock.sendto(connReq, (host, port))
        resp1 = sock.recv(65536)
    except socket.error as err:
        return None, '', ''

    infoReq = ''.join([
        struct.pack('>I', 1),  # msgid
        '\x00' * 4,  # msgcall
        '\x00\x00\x00\x02',  # rpc version
        '\x55' * 4,  # wdb programe number
        '\x00\x00\x00\x01',  # programe version
        struct.pack('>I', 123),  # function number: WDB_TGT_INFO_GET = 123
        '\x00' * 16,
        '\x00' * 4,
        '\x00\x00\x00\x44',  # packet length
        struct.pack('>I', 1),  # msg seq
        '\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00',  # parameter
    ])
    try:
        sock.sendto(infoReq, (host, port))
        resp2 = sock.recv(65536)
    except socket.timeout as err:
        resp2 = ''

    return 'vxworks' in resp2.lower(), resp1, resp2


if __name__ == '__main__':
    import sys
    from pprint import pprint as pr

    pr(scanV1(sys.argv[1]))
    print
    pr(scanV2(sys.argv[2]))
