#!/usr/bin/env python
# encoding: utf-8


'''
# Sulley Fuzzing Script: rpcbind / udp port 111
# Author: dog2
# Reference:
#     1. UNP vol2 v2 ch16
#     2. http://docs.oracle.com/cd/E19683-01/816-1435/rpcproto-13077/index.html
'''

import sys
import json
import time
import signal
import struct
import logging

import wdbdbg

from sulley import *

L4_PROTOCOL = 'udp'  # layer 4 protocol, 'tcp' or 'udp'
PORT = 111
ENDIAN = BIG_ENDIAN


######################################################################
#########################
##  1 - common fields  ##
#########################

s_initialize('r_common_fileds')  # r_xyz: request xyz

# transaction identifier
# xid
# 4 bytes
# unsigned int
s_int(
    value=0x72fe1d13,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_xid',  # f_xyz: field xyz
)

# message type
# mtype
# 4 bytes
# enum
MSG_TYPE_CALL = 0
MSG_TYPE_REPLY = 1
s_int(
    value=MSG_TYPE_CALL,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_mtype',
)

# rpc version
# rpcvers
# 4 bytes
# unsigned int
s_int(
    value=2,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_rpcvers',
)

# remote program
# prog
# 4 bytes
# unsigned int
s_int(
    value=100000,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_prog',
)

# remote program version
# vers
# 4 bytes
# unsigned int
s_int(
    value=104316,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_vers',
)

# the procedure within the remote program to be called
# proc
# 4 bytes
# unsigned int
s_int(
    value=0,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_proc',
)
######################################################################



######################################################################
#######################
##  2 - auth fields  ##
#######################

AUTH_FLAVORS = {
    'AUTH_NONE': 0,
    'AUTH_SYS': 1,
    'AUTH_SHORT': 2,
    'AUTH_DES': 3,
    'AUTH_KERB': 4,
}


#########################
##  2.1 - cred fields  ##
#########################

s_initialize('r_cred_flavor_fuzz')
# cred flavor(fuzz)
# cred_flavor
# 4 bytes
# unsigned int
s_int(
    value=0,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_cred_flavor_fuzz',
)

s_initialize('r_cred_flavor')
# cred flavor
# cred_flavor
# 4 bytes
# unsigned int
s_int(
    value=AUTH_FLAVORS.values(),
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_cred',
)

s_initialize('r_cred_body')
# cred length
# 4 bytes
# unsigned int
s_int(
    value=0,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_cred_len',
)
# cred data
# variable length, max 400 bytes
s_random(
    value='\x00\x00\x00\x00',
    min_length=0,
    max_length=500,
    num_mutations=25,
    fuzzable=True,
    step=None,
    name='f_cred_data',
)



#########################
##  2.2 - verf fields  ##
#########################
s_initialize('r_verf_flavor_fuzz')
# verf flavor(fuzz)
# verf_flavor
# 4 bytes
# unsigned int
s_int(
    value=0,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_verf_flavor_fuzz',
)

s_initialize('r_verf_flavor')
# verf flavor
# verf_flavor
# 4 bytes
# unsigned int
s_int(
    value=AUTH_FLAVORS.values(),
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_verf_flavor',
)

s_initialize('r_verf_body')
# verf length
# 4 bytes
# unsigned int
s_int(
    value=0,
    endian=ENDIAN,
    format='binary',
    signed=False,
    full_range=False,
    fuzzable=True,
    name='f_verf_len',
)
# verf data
# variable length, max 400 bytes
s_random(
    value='\x00\x00\x00\x00',
    min_length=0,
    max_length=500,
    num_mutations=25,
    fuzzable=True,
    step=None,
    name='f_verf_data',
)

######################################################################


######################################################################
#####################
##  3 - proc args  ##
#####################
s_initialize(name='r_proc_args')
# procedure arguments
# variable length
s_random(
    value=None,
    min_length=0,
    max_length=1024,
    num_mutations=128,
    fuzzable=True,
    step=None,
    name='f_proc_args',
)



class MySession(sessions.session):
    def __init__(self, session_filename=None, skip=0, sleep_time=1.0,
                log_level=logging.INFO, logfile=None,
                logfile_level=logging.DEBUG, proto="tcp",
                bind=None, restart_interval=0, timeout=5.0,
                web_port=26000, crash_threshold=3,
                restart_sleep_time=300, vx_version='5.5'):
        super(MySession, self).__init__(
            session_filename, skip, sleep_time,
            log_level, logfile,
            logfile_level, proto,
            bind, restart_interval, timeout,
            web_port, crash_threshold,
            restart_sleep_time
        )
        self.vx_version = vx_version

    def pre_send (self, sock):
        if not hasattr(self, 'dbg'):
            if self.vx_version == '5.5':
                self.dbg = wdbdbg.VXWORKS55(
                    Dstip=self.targets[0].host,
                    Dstport=17185,
                    OutTime=3
                )
            elif self.vx_version == '6.6':
                self.dbg = wdbdbg.VXWORKS66(
                    Dstip=self.targets[0].host,
                    Dstport=17185,
                    OutTime=3
                )
        self.logger.info(self.targets[0].host)
        self.dbg.pub_Connect()

    def post_send(self, sock):
        time.sleep(0.5)
        bok, excepinfo = self.dbg.pub_IsAbort()
        if not bok:
            self.logger.info('no error!')
            return
        else:
            self.logger.info('found error! Restarting Vxworks, sleeping for 10s...')
            f = open('crash.log', 'ab')
            excepinfo.update({'index': self.total_mutant_index,})
            f.write(
                json.dumps(excepinfo) + '\n',
            )
            f.close()

            # retrieve the primitive that caused the crash and increment it's individual crash count.
            self.crashing_primitives[self.fuzz_node.mutant] = self.crashing_primitives.get(self.fuzz_node.mutant, 0) + 1

            # notify with as much information as possible.
            if self.fuzz_node.mutant.name:
                msg = "primitive name: %s, " % self.fuzz_node.mutant.name
            else:
                msg = "primitive lacks a name, "

            msg += "type: %s, default value: %s" % (self.fuzz_node.mutant.s_type, self.fuzz_node.mutant.original_value)
            self.logger.info(msg)

            if self.crashing_primitives[self.fuzz_node.mutant] >= self.crash_threshold:
                # as long as we're not a group and not a repeat.
                if not isinstance(self.fuzz_node.mutant, primitives.group):
                    if not isinstance(self.fuzz_node.mutant, blocks.repeat):
                        skipped = self.fuzz_node.mutant.exhaust()
                        self.logger.warning("crash threshold reached for this primitive, exhausting %d mutants." % skipped)
                        self.total_mutant_index += skipped
                        self.fuzz_node.mutant_index += skipped

            self.dbg.pub_DisConnect()
            time.sleep(14)
            return



def do_fuzz(dstIp, vx_version='5.5'):
    s = MySession(
        session_filename='audits/vx66_udp111rpcbind/rpcbind.session',
        skip=0,
        sleep_time=1,
        log_level=logging.INFO,
        logfile=None,
        logfile_level=logging.DEBUG,
        proto=L4_PROTOCOL,
        bind=None,
        restart_interval=0,
        timeout=2.0,
        web_port=26000,
        crash_threshold=3,
        restart_sleep_time=60,
        vx_version=vx_version,
    )

    s.connect(s.root, s_get(name='r_common_fileds'))

    s.connect(s_get(name='r_common_fileds'), s_get(name='r_cred_flavor_fuzz'))
    s.connect(s_get(name='r_cred_flavor_fuzz'), s_get(name='r_cred_body'))
    s.connect(s_get(name='r_cred_body'), s_get(name='r_verf_flavor'))
    s.connect(s_get(name='r_verf_flavor'), s_get(name='r_verf_body'))

    s.connect(s_get(name='r_cred_body'), s_get(name='r_verf_flavor_fuzz'))
    s.connect(s_get(name='r_verf_flavor_fuzz'), s_get(name='r_verf_body'))


    s.connect(s_get(name='r_common_fileds'), s_get(name='r_cred_flavor'))
    s.connect(s_get(name='r_cred_flavor'), s_get(name='r_cred_body'))
    s.connect(s_get(name='r_cred_body'), s_get(name='r_verf_flavor'))
    s.connect(s_get(name='r_verf_flavor'), s_get(name='r_verf_body'))

    s.connect(s_get(name='r_cred_body'), s_get(name='r_verf_flavor_fuzz'))
    s.connect(s_get(name='r_verf_flavor_fuzz'), s_get(name='r_verf_body'))

    s.connect(s_get(name='r_verf_body'), s_get(name='r_proc_args'))

    print s.num_mutations()
    #fh = open("rpcbind.udg", "wb")
    #fh.write(s.render_graph_udraw())
    #fh.close()

    target = sessions.target(dstIp, PORT)
    target.netmon  = pedrpc.client("192.168.102.128", 26001)
    #target.procmon = pedrpc.client("10.1.1.182", 26002)
    #target.procmon_options =  {
        #"proc_name"      : "SlimFTPd.exe",
        #"stop_commands"  : ['wmic process where (name="SlimFTPd.exe") delete'],
        #"start_commands" : ['C:\\Users\\eleanor\\Desktop\\slimftp3.15b\\SlimFTPd.exe'],
        #"start_commands" : ['C:\\Documents and Settings\\eleanor\\Desktop\\slimftp\\slimftp3.15\\SlimFTPd.exe'],
    #}
    s.add_target(target)
    s.fuzz()


if __name__ == '__main__':
    #do_fuzz(dstIp='192.168.102.88', vx_version='5.5')
    do_fuzz(dstIp='192.168.102.89', vx_version='6.6')
