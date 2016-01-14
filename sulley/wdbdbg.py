#!/usr/bin/env python
# encoding: utf-8

# author: mt/xiaohu@404

import struct
import socket
import string
import time
import threading

from capstone import *

#public interface

#dst infomation
class VXWORKS55(object):

    def __init__(self, Dstip, Dstport = 17185, OutTime = 5):

        self.gDstip = Dstip
        self.gDstport = Dstport
        self.gSock = None
        self.gMsgid = 0x11112222
        self.gMsgSeq = 0x33334444
        self.gByteBigorder = False
        self.REQUEST_RPC_HEAD_SIZE = 52
        self.REPLAY_RPC_HEAD_SIZE = 36
        self.gOutTime = OutTime

        self.gSock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        self.gSock.settimeout(OutTime)


    def __del__(self):

        if self.gSock != None:
            self.gSock.close()

        self.gDstip = ''
        self.gDstport = 0
        self.gSock = None
        self.gMsgid = 0
        self.gMsgSeq = 0
        self.gByteBigorder = False
        self.REQUEST_RPC_HEAD_SIZE = 0
        self.REPLAY_RPC_HEAD_SIZE = 0
        self.gOutTime = 0



    def __Corealigned4(self, x):

        mod = x % 4

        if mod == 0:
            return x

        x = (4 - mod) + x
        return x




    def __CoreSend(self, data):

        bret = False

        try:
            sendlenght = self.gSock.sendto(data, (self.gDstip, self.gDstport))
            datalength = len(data)
            if(sendlenght == datalength):
                bret = True

        except socket.timeout:
            bret =  False
        except Exception as be:
            bret = False

        return bret


    def __CoreRecvData(self):

        bret = False
        data = ""

        try:
            data, dst = self.gSock.recvfrom(4096)
            if(len(data) > 0):
                bret = True

        except socket.timeout:
            bret =  False
        except Exception as be:
            bret = False

        return bret,data


    def __CoreChksum(self, data):

        chksum = 0
        return chksum

    def __Request(self, RId, parameter = '', bRecvData = True):

        bok = False
        data = ''
        packlen = len(parameter) + 0x30

        requestid = struct.pack('>I', RId)

        data = data + struct.pack('>I', self.gMsgid)     #msgid
        data = data + 4*'\x00'                      #msgcall
        data = data + '\x00\x00\x00\x02'            #rpc version
        data = data + 4*'\x55'                      #wdb programe number
        data = data + '\x00\x00\x00\x01'            #programe version
        data = data + requestid                     #function nmber
        data = data + 16*'\x00'

        data = data + 4*'\x00'                      #checksum
        data = data + struct.pack('>I', packlen)    #pack length
        data = data + struct.pack('>I', self.gMsgSeq)    #msg seq
        data = data + parameter

        self.__CoreChksum(data)

        repeatRecv = 0

        bok = self.__CoreSend(data)

        if bok == False:
            bok = False
            return bok,None

        if bRecvData == False:
            bok = True
            return bok,None

        while repeatRecv < 3:
            bok, data = self.__CoreRecvData()

            if bok == True:
                rpchead = data[:36]
                msgid = struct.unpack('>I', rpchead[:4])[0]
                if msgid != self.gMsgid:
                    continue

                rpchead = rpchead[4:]
                data = data[36:]

                msgreply,a1,a2,a3,a4,chk,size,wdbstatus = struct.unpack(">IIIIIIII", rpchead)
                if wdbstatus !=0:
                    bok = False
                else:
                    bok = True

                break

        if bok == True:
            self.gMsgid = self.gMsgid + 1
            self.gMsgSeq = self.gMsgSeq + 1

        return bok, data

    def PushConnect(self, DstIp, DstPort = 17185, TimeOut = 5):
        """"
        803c7450	msgid
        00000000	msgcall
        00000002
        55555555
        00000001
        00000001 	wdb_target_connect

        00000000
        00000000
        00000000
        00000000

        ffff4780
        00000030
        0da00001	msgseq
        --------

        803c7450	msgid
        00000001	msgreply
        00000000
        00000000
        00000000
        00000000

        ffff4e91 	chk
        00000094 	size
        00000000 	wdb status

        WDB_TGT_INFO
                wdb_agent_info
        00000004	wersion of the wdb agent
        322e3000	"2.0"
        000005dc	mtu:maximum transfer size in bytes
        00000003	mode:svailable agent modes

                wdb_rt_info
        00000001	runtime type
        0000000b	runtime version
        5678576f726b73352e350000 "vxworks5.5"

        00000053	target processor type
        00000001	target has a floating point unit
        00000000	target can write  protect memory
        00001000	size of a page
        000010e1	endianess(4321 or 1234)

        0000000b	bspname
        50432050454e5449554d0000 "pcpentium"
        0000000d	bootline
        686f73743a7678576f726b7300000000 "host:vxworks"

        00100000	memBase:target main memory base address
        00f00000	target main memory size
        00000000	number of memory regions
        00000000	memory regions descriptors
        003f94a0	hostPoolBase :  host-controlled tgt memory pool
        000c06b6	hostPoolsize
        """
        bok = False
        data = ''
        mod = 0


        try:

            bok, data = self.__Request(1)   #wdb_target_connect = 1

            if bok == True:
                wdbversionlen = self.__Corealigned4(struct.unpack('>I', data[:4])[0])
                data = data[4:]
                wdbversion = data[:wdbversionlen]
                data = data[wdbversionlen:]

                maxTransefersize = struct.unpack('>I', data[:4])[0]
                data = data[4:]

                agentmodes = struct.unpack('>I', data[:4])[0]
                data = data[4:]

                runtimetype = struct.unpack('>I', data[:4])[0]
                data = data[4:]

                runtimeverlen = self.__Corealigned4(struct.unpack('>I', data[:4])[0])
                data = data[4:]
                runtimever = data[:runtimeverlen]
                data = data[runtimeverlen:]


                TargetProcessorType = struct.unpack('>I',data[:4])[0]
                data = data[4:]

                targethasfloatingunit = struct.unpack('>I',data[:4])
                data = data[4:]

                targetcanwriteprotectmem = struct.unpack('>I',data[:4])
                data = data[4:]

                sizeofpage = struct.unpack('>I',data[:4])
                data = data[4:]

                endianess = struct.unpack('>I',data[:4])             #4321 or 1234
                data = data[4:]
                if endianess == '4321':
                    self.gByteBigorder = False
                else:
                    self.gByteBigorder = True


                bspnamelen = self.__Corealigned4(struct.unpack('>I', data[:4])[0])
                data = data[4:]
                bspname = data[:bspnamelen]
                data = data[bspnamelen:]

                bootlinelen =  self.__Corealigned4(struct.unpack('>I', data[:4])[0])
                data = data[4:]
                bootline = data[:bootlinelen]
                data = data[bootlinelen:]

                targetmainmembaseaddr = struct.unpack('>I',data[:4])
                data = data[4:]

                targetmainmemsize = struct.unpack('>I',data[:4])
                data = data[4:]

                numberofmemregions = struct.unpack('>I',data[:4])
                data = data[4:]

                memregdescriptiors = struct.unpack('>I',data[:4])
                data = data[4:]

        except Exception as e:
            bok = False


        return bok, data


    def PushDisConnect(self):
        bok = False
        data = ''
        parameter = ''

        """     input:
        none
        """
        try:

            bok, data = self.__Request(2, parameter)  #WDB_TARGET_DISCONNECT

            """     output:
            none
            """
        except Exception as e:
            bok = False

        return bok,None

    def PushRebootTarget(self):
        bok = False
        data = ''
        parameter = ''

        """     input: WDB_CTX
        {
        WDB_CTX_TYPE	contextType;	/* type of context */
        UINT32		contextId;	/* context ID */
        }
        """
        try:
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0)

            bok, data = self.__Request(0x1f, parameter, False)  #WDB_CONTEXT_KILL

            """     output:
            none
            """
        except Exception as e:
            bok = False

        return bok,None

    def PushReadMemory(self, baseAddr, numBytes):

        bok = False
        data = ''
        parameter = ''

        """   input:
            WDB_MEM_REGION                          /* a region of target memory */
            {
            TGT_ADDR_T		baseAddr;	/* memory region base address */
            TGT_INT_T		numBytes;	/* memory region size */
            UINT32		        param;		/* proc dependent parameter */
            } ;
        """
        try:
            parameter = parameter + struct.pack('>I', baseAddr)
            parameter = parameter + struct.pack('>I', numBytes)
            parameter = parameter + struct.pack('>I', 0)

            bok, data = self.__Request(10, parameter)   #WDB_MEM_READ = 10

            """     output:
            WDB_MEM_XFER         		/* transfer a block of memory */
            {
            WDB_OPQ_DATA_T	source;		/* data to transfer */
            TGT_ADDR_T		destination;	/* requested destination */
            TGT_INT_T		numBytes;	/* number of bytes transferred */
            } ;

            """

            if bok == True:
                param = data[0:12]
                data = data[12:]

                source,destination,numBytee = struct.unpack(">III", param)
        except Exception as e:
            bok = False



        return bok, data


    def PushWriteMemory(self, baseAddr, numBytes, indata):

        bok = False
        data = ''
        parameter = ''

        """     input:
        WDB_MEM_XFER         		/* transfer a block of memory */
        {
        WDB_OPQ_DATA_T	source;		/* data to transfer */
        TGT_ADDR_T		destination;	/* requested destination */
        TGT_INT_T		numBytes;	/* number of bytes transferred */
        } ;

        """
        try:
            parameter = parameter + struct.pack('>I', numBytes)
            parameter = parameter + struct.pack('>I', baseAddr)
            parameter = parameter + struct.pack('>I', numBytes)
            parameter = parameter + indata

            bok, data = self.__Request(11, parameter)   #WDB_MEM_WRITE

            """     output:
            none
            """

            if bok == True:
                param = data[0:12]
                data = data[12:]

                source,destination,numBytee = struct.unpack(">III", param)
        except Exception as e:
            bok = False



        return bok, data


    def PushRedRegister(self, taskid):

        bok = False
        data = ''
        parameter = ''
        regsvar = ''
        regformat = ''

        """     input:
        56b88743
        00000000
        00000002
        55555555
        00000001
        00000028		WDB_REGS_GET
        00000000
        00000000
        00000000
        00000000

        ffff90aa
        00000048
        0734004e
                                in:WDB_REG_READ_DESC
        00000000
        00000003
        00f6bb94
        00000000
        00000028
        00000000

            //WDB_REG_READ_DESC					/* register data to read */
        {
        WDB_REG_SET_TYPE 	regSetType;				/* type of register set to read */
        WDB_CTX	     	context;				/* context associated with registers */
        WDB_MEM_REGION   	memRegion;				/* subregion of the register block */
        } ;

            //WDB_CTX						/* a particular context */
        {
        WDB_CTX_TYPE		contextType;			/* type of context */
        UINT32			contextId;		/* context ID */
        } ;

            //WDB_REG_SET_TYPE   			/* a type of register set */
        {
        WDB_REG_SET_IU	= 0,            /* integer unit register set */
        WDB_REG_SET_FPU	= 1,            /* floating point unit register set */
        WDB_REG_SET_MMU	= 2,            /* memory management unit reg set */
        WDB_REG_SET_CU	= 3,            /* cache unit register set */
        WDB_REG_SET_TPU	= 4,            /* timer processor unit register set */
        WDB_REG_SET_SYS	= 5,		/* system registers */
        WDB_REG_SET_DSP	= 6,		/* dsp registers */
        WDB_REG_SET_AV	= 7,		/* altivec registers */
        WDB_REG_SET_LAST	= 8		/* 1 + last used REG_SET number */
        } ;

            //WDB_CTX_TYPE				/* type of context on the target */
        {
        WDB_CTX_SYSTEM	= 0000,		/* system mode */
        WDB_CTX_GROUP	= 0001,		/* process group (not implemented) */
        WDB_CTX_ANY		= 0002,		/* any context (not implemented) */
        WDB_CTX_TASK	= 0003,		/* specific task or processes */
        WDB_CTX_ANY_TASK	= 0004,		/* any task */
        WDB_CTX_ISR		= 0005,		/* specific ISR (not implemented) */
        WDB_CTX_ANY_ISR	= 0006		/* any ISR (not implemented) */
        } ;
            //WDB_MEM_REGION			/* a region of target memory */
        {
        TGT_ADDR_T		baseAddr;	/* memory region base address */
        TGT_INT_T		numBytes;	/* memory region size */
        UINT32		param;		/* proc dependent parameter */
        } ;

        """
        try:
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 3)
            parameter = parameter + struct.pack('>I', taskid)
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0x28)
            parameter = parameter + struct.pack('>I', 0)

            bok, data = self.__Request(0x28, parameter)   #WDB_REGS_GET

            """     output:
            --------
            56b88743
            00000001
            00000000
            00000000
            00000000
            00000000
            fffff87b
            00000054
            00000000
                                            out:WDB_MEM_REGION
            00000028
            00000004
            00000028

            01000000		edi
            b4b8f600		esi
            64b8f600		ebp
            44b8f600		esp
            d4b9f600		ebx
            64ddff00		edx
            b4b9f600		ecx
            88888888		eax
            06020000		efalgs
            9c473900		pc
            """
            'edi,esi,ebp,esp,ebx,edx,ecx,eac,efalgs,pc'
            if bok == True:
                outparam = data[0:12]
                data = data[12:]

                baseaddr,numBytee,param = struct.unpack(">III", outparam)
        except Exception as e:
            bok = False



        return bok, data

    def PushGetExcepData(self):
        bok = False
        data = ''
        parameter = ''
        evecounts = 0

        """     input:
        none
        """
        try:

            bok, data = self.__Request(0x46, parameter)   #WDB_EVENT_GET

            """     output:WDB_EVT_DATA
            {
              WDB_EVT_TYPE
              WDB_EVT_INFO
            } ;

                {
                WDB_EVT_NONE	= 0,		/* no event */
                WDB_EVT_CTX_START	= 1,		/* context creation */
                WDB_EVT_CTX_EXIT	= 2,		/* context exit */
                WDB_EVT_BP		= 3,		/* breakpoint */
                WDB_EVT_HW_BP	= 4,		/* hardware breakpoint */
                WDB_EVT_WP		= 5,		/* watchpoint */
                WDB_EVT_EXC		= 6,		/* exception */
                WDB_EVT_VIO_WRITE	= 7,		/* virtual I/O write */
                WDB_EVT_CALL_RET	= 8,		/* function call finished */
                WDB_EVT_TSFS_OP	= 9,		/* VIO II operation */
                WDB_EVT_USER	= 10		/* user event */
                } WDB_EVT_TYPE;


                        /* 0 - 19	exception vector number */

                #define IN_DIVIDE_ERROR			0
                #define IN_DEBUG			1
                #define IN_NON_MASKABLE			2
                #define IN_BREAKPOINT			3
                #define IN_OVERFLOW			4
                #define IN_BOUND			5
                #define IN_INVALID_OPCODE		6
                #define IN_NO_DEVICE			7
                #define IN_DOUBLE_FAULT			8
                #define IN_CP_OVERRUN			9
                #define IN_INVALID_TSS			10
                #define IN_NO_SEGMENT			11
                #define IN_STACK_FAULT			12
                #define IN_PROTECTION_FAULT		13
                #define IN_PAGE_FAULT			14
                #define IN_RESERVED			15
                #define IN_CP_ERROR			16
                #define IN_ALIGNMENT			17
                #define IN_MACHINE_CHECK		18
                #define IN_SIMD				19

            """

            if bok == True and len(data) > 8:
                evetype,evecounts = struct.unpack('>II', data[:8])
                data = data [8:]

                if evetype == 6:
                    bok = True


                else:
                    print "===============================unknown event=======================================\n"
                    print "evetype = %d evecounts = %d " % (evetype, evecounts)
                    i = 0
                    while (i < evecounts):
                        value = struct.unpack('>I', data[i*4:i*4+4])[0]
                        print "array[%d] = %x" % (i, value)
                        i = i + 1
                    print "===================================================================================\n"
                    bok = False
            else:
                bok = False

        except Exception as e:
            bok = False

        return bok, evecounts, data


    def pub_Connect(self):
        bok = False
        data = ''


        bok,data = self.PushConnect(self.gDstip, self.gDstport, self.gOutTime)
        '''
        if bok ==True:
            gThreadRecvExcepEvent = threading.Thread(target = ThreadWork, name = "ThreadWork")

        gThreadRecvExcepEvent.start()
        '''
        return bok

    def pub_DisConnect(self):

        bok = False

        #PushDisConnect()
        bok = self.PushRebootTarget()
        #if self.gSock != None:
            #self.gSock.close()

        #self.gSock = None

        return bok


    def pub_IsAbort(self):

        excepinfo = {}
        excepstr = ['DIVIDE_ERROR', 'DEBUG', 'NON_MASKABLE', 'BREAKPOINT', 'OVERFLOW', 'BOUND', 'INVALID_OPCODE', 'NO_DEVICE', 'DOUBLE_FAULT', 'CP_OVERRUN', 'INVALID_TSS', 'NO_SEGMENT', 'STACK_FAULT', 'PROTECTION_FAULT', 'PAGE_FAULT', 'RESERVED', 'CP_ERROR', 'ALIGNMENT', 'MACHINE_CHECK', 'SIMD']

        eventmap = {}
        regmap = {}
        taskid = 0
        data = ''

        #if gOcuExcepEvent == False:
        #    return False,None


        #get event
        bok,evecount,data = self.PushGetExcepData()
        if bok == False:
            return False,None

        i = 0
        while i < evecount / 2:
            n = struct.unpack('>I', data[:4])[0]
            key = excepstr[n]
            data = data[4:]

            value = struct.unpack('>I', data[:4])[0]
            data = data[4:]

            eventmap[key] = value

            if n == 3:  #WDB_CTX_TYPE   in   WDB_CTX_TASK	= 0003,		/* specific task or processes */ ???????
                taskid = value
            i = i + 1

        #get reg
        if taskid != 0:
            bok, data = self.PushRedRegister(taskid)
        if bok == False:
            return bok, None

        #gByteBigorder == false
        edi,esi,ebp,esp,ebx,edx,ecx,eax,eflags,pc = struct.unpack('<IIIIIIIIII', data)
        regmap['edi'] = edi
        regmap['esi'] = esi
        regmap['ebp'] = ebp
        regmap['esp'] = esp
        regmap['ebx'] = ebx
        regmap['edx'] = edx
        regmap['ecx'] = ecx
        regmap['eax'] = eax
        regmap['eflags'] = eflags
        regmap['pc'] = pc

        #get code
        bok,data = self.PushReadMemory(pc, 100)
        asmcode = ''

        #depend capstone
        #http://www.capstone-engine.org/download.html
        #Python module for Windows 32 - Binaries
        if bok == True and len(data) > 0:
            md = Cs(CS_ARCH_X86, CS_MODE_32)
            for i in md.disasm(data, pc):
                asmcode =  asmcode + ("0x%x:\t%s\t%s\n" % (i.address, i.mnemonic, i.op_str))

        excepinfo['eve'] = eventmap
        excepinfo['reg'] = regmap
        excepinfo['asm'] = asmcode


        return bok, excepinfo





class VXWORKS66(object):
    '''
    reference: \WindRiver-GPPVE-3.6-IA-Eval\vxworks-6.6\target\h\wdb\wdb.h
    '''
    def __init__(self, Dstip, Dstport = 17185, OutTime = 5, doChksum=False):

        self.gDstip = Dstip
        self.gDstport = Dstport
        self.gSock = None
        self.gMsgid = 0x11112222
        self.gMsgSeq = 0x33334444
        self.gByteBigorder = False
        self.REQUEST_RPC_HEAD_SIZE = 52
        self.REPLAY_RPC_HEAD_SIZE = 36
        self.gOutTime = OutTime
        self.doChksum = doChksum

        self.gSock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        self.gSock.settimeout(OutTime)


    def __del__(self):

        if self.gSock != None:
            self.gSock.close()

        self.gDstip = ''
        self.gDstport = 0
        self.gSock = None
        self.gMsgid = 0
        self.gMsgSeq = 0
        self.gByteBigorder = False
        self.REQUEST_RPC_HEAD_SIZE = 0
        self.REPLAY_RPC_HEAD_SIZE = 0
        self.gOutTime = 0



    def __Corealigned4(self, x):

        mod = x % 4

        if mod == 0:
            return x

        x = (4 - mod) + x
        return x




    def __CoreSend(self, data):

        bret = False

        try:
            sendlenght = self.gSock.sendto(data, (self.gDstip, self.gDstport))
            datalength = len(data)
            if(sendlenght == datalength):
                bret = True

        except socket.timeout:
            bret =  False
        except Exception as be:
            bret = False

        return bret


    def __CoreRecvData(self):

        bret = False
        data = ""

        try:
            data, dst = self.gSock.recvfrom(4096)
            if(len(data) > 0):
                bret = True

        except socket.timeout:
            bret =  False
        except Exception as be:
            bret = False

        return bret,data


    def CoreChksum(self, data):

        chksum = 0

        rdata = data[4:]
        rdata = data
        rlen = len(rdata)
        cos = rlen % 2
        count = rlen / 2


        while( count > 0):
            ndata = rdata[0:2]
            n = struct.unpack('>H',ndata)[0]
            rdata = rdata[2:]
            count = count - 2
            chksum = chksum + n

        lw = chksum & 0xffff
        hw = chksum >> 16
        chksum = lw + hw

        lw = chksum & 0xffff
        hw = chksum >> 16
        chksum = lw + hw


        chksum = ~chksum

        print "%08x" % chksum

        return chksum

    def _Request(self, RId, parameter = '', bRecvData = True):

        bok = False
        data = ''
        packlen = len(parameter) + 0x30

        requestid = struct.pack('>I', RId)

        data = data + struct.pack('>I', self.gMsgid)     #msgid
        data = data + 4*'\x00'                      #msgcall
        data = data + '\x00\x00\x00\x02'            #rpc version
        data = data + 4*'\x55'                      #wdb programe number
        data = data + '\x00\x00\x00\x01'            #programe version
        data = data + requestid                     #function nmber
        data = data + 16*'\x00'

        data1 = data + 4*'\x00'                      #checksum
        end = struct.pack('>I', packlen)    #pack length
        end += struct.pack('>I', self.gMsgSeq)    #msg seq

        data1 = data1 + end + parameter
        data = data + struct.pack('>i', self.CoreChksum(data1)) + end + parameter

        repeatRecv = 0

        if self.doChksum:
            bok = self.__CoreSend(data)
        else:
            bok = self.__CoreSend(data1)

        if bok == False:
            bok = False
            return bok,None

        if bRecvData == False:
            bok = True
            return bok,None

        while repeatRecv < 3:
            bok, data = self.__CoreRecvData()

            if bok == True:
                rpchead = data[:36]
                msgid = struct.unpack('>I', rpchead[:4])[0]
                if msgid != self.gMsgid:
                    continue

                rpchead = rpchead[4:]
                data = data[36:]

                msgreply,a1,a2,a3,a4,chk,size,wdbstatus = struct.unpack(">IIIIIIII", rpchead)
                if wdbstatus !=0:
                    bok = False
                else:
                    bok = True

                break

        if bok == True:
            self.gMsgid = self.gMsgid + 1
            self.gMsgSeq = self.gMsgSeq + 1

        return bok, data

    def PushConnect(self, DstIp, DstPort = 17185, TimeOut = 5):
        """input:
        none
        """
        bok = False
        data = ''
        mod = 0

        try:

            bok, data = self._Request(122)   #WDB_TARGET_CONNECT2 = 122

            if bok == True:
                """output: WDB_CONNECT_INFO
                 				/* reply to WDB_TARGET_CONNECT2 */
               {
               WDB_AGENT_INFO	agentInfo;	/* info about the agent 	*/
               WDB_STRING_T	rtName;		/* runtime name 		*/
               UINT32	 	tgtLongSize;	/* sizeof(long)  on target	*/
               UINT32	 	tgtAddrSize;   	/* sizeof(void*) on target	*/
               UINT32		tgtIntSize;	/* sizeod(int) on target	*/
               } ;

                {WDB_AGENT_INFO
                WDB_STRING_T	agentVersion;	/* version of the WDB agent */
                TGT_INT_T		mtu;		/* maximum transfer size in bytes */
                TGT_INT_T		mode;		/* available agent modes */
                TGT_INT_T		options;	/* available agent options */
                } ;

                """

        except Exception as e:
            bok = False

        return bok, data


    def PushDisConnect(self):
        bok = False
        data = ''
        parameter = ''

        """     input:
        none
        """
        try:

            bok, data = self._Request(2, parameter)  #WDB_TARGET_DISCONNECT

            """     output:
            none
            """
        except Exception as e:
            bok = False

        return bok,None


    def PushRebootTarget(self):
        bok = False
        data = ''
        parameter = ''

        """     input: WDB_CTX
        {
        WDB_CTX_TYPE	contextType;	/* type of context */
        UINT32		contextId;	/* context ID */
        }
        """
        try:
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0)

            bok, data = self._Request(0x1f, parameter, False)  #WDB_CONTEXT_KILL

            """     output:
            none
            """
        except Exception as e:
            bok = False

        return bok,None

    def PushReadMemory(self, baseAddr, numBytes):

        bok = False
        data = ''
        parameter = ''

        """   input:
        c078b99a
        00000000
        00000002
        55555555
        00000001
        0000000a
        00000000
        00000000
        00000000
        00000000

        ffff474b
        00000050
        0974056a

                WDB_CORE:
        00000003
        00e715b8
        00000000
        7c92da0c

                WDB_MEM_REGION:
        00000000
        0081de64
        00000080
        00e7b140

            WDB_MEM_REGION                          /* a region of target memory */
            {
            TGT_INT_T           options;        /* memory options */
            TGT_ADDR_T		baseAddr;	/* memory region base address */
            TGT_INT_T		numBytes;	/* memory region size */
            UINT32		param;		/* proc dependent parameter */
            } ;
        """
        try:
            #wdb_core
            parameter = parameter + struct.pack('>I', 2)
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0)

            #wdb_mem_region
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', baseAddr)
            parameter = parameter + struct.pack('>I', numBytes)
            parameter = parameter + struct.pack('>I', 0)

            bok, data = self._Request(10, parameter)   #WDB_MEM_READ = 10

            """     output:
            WDB_MEM_XFER         		/* transfer a block of memory */
            {
            TGT_INT_T           options;        /* memory options */
            WDB_OPQ_DATA_T	source;		/* data to transfer */
            TGT_ADDR_T		destination;	/* requested destination */
            TGT_INT_T		numBytes;	/* number of bytes transferred */
            } ;

            """

            if bok == True:
                param = data[0:16]
                data = data[16:]

                options,source,destination,numBytee = struct.unpack(">IIII", param)
        except Exception as e:
            bok = False



        return bok, data


    def PushWriteMemory(self, baseAddr, numBytes, indata):

        bok = False
        data = ''
        parameter = ''

        """     input:
        WDB_CORE:

        WDB_MEM_XFER         		/* transfer a block of memory */
        {
        TGT_INT_T               options;        /* memory options */
        WDB_OPQ_DATA_T	        source;		/* data to transfer */
        TGT_ADDR_T		destination;	/* requested destination */
        TGT_INT_T		numBytes;	/* number of bytes transferred */
        } ;

        """
        try:
            #wdb_core
            parameter = parameter + struct.pack('>I', 2)
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0)

            #wdb_mem_xfer
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', numBytes)
            parameter = parameter + struct.pack('>I', baseAddr)
            parameter = parameter + struct.pack('>I', numBytes)
            parameter = parameter + indata
            bok, data = self._Request(11, parameter)   #WDB_MEM_WRITE

            """     output:
            none
            """
        except Exception as e:
            print e.message
            bok = False



        return bok, data


    def PushReadRegister(self, taskid):

        bok = False
        data = ''
        parameter = ''
        regsvar = ''
        regformat = ''

        """     input:
        41757b70
        00000000
        00000002
        55555555
        00000001
        00000028               WDB_REGS_GET

        00000000
        00000000
        00000000
        00000000

        ffffd978
        00000064
        09bc01eb

        wdb_core:

        00000003                wdb_code.numints
        00f4b480                wdb_code.toolid
        00000000                wdb_code.memCtx.Contextid
        71a2265b                wdb_code.memCtx.pointerSize

        WDB_REG_READ_DESC:

        00000000		regSetType
        00000003		wdb_ctx.contextType #specific task or processes
        00000001		wdb_ctx.numArgs
        00000001		wdb_ctx.args
        007c218c

        00000000                WDB_MEM_REGION.option
        00000000                WDB_MEM_REGION.baseAddr
        00000024                WDB_MEM_REGION.numbytes
        00000000                WDB_MEM_REGION.param

            //WDB_REG_READ_DESC					/* register data to read */
        {
        WDB_REG_SET_TYPE 	regSetType;				/* type of register set to read */
        WDB_CTX	     	context;				/* context associated with registers */
        WDB_MEM_REGION   	memRegion;				/* subregion of the register block */
        } ;

            //WDB_CTX						/* a particular context */
        {
        WDB_CTX_TYPE	contextType;	/* type of context */
        UINT32		numArgs;	/* structure length: it depends on */
                                        /* context type and OS */
        TGT_LONG_T *	args;		/* information to characterize */
        } ;

            //WDB_REG_SET_TYPE   			/* a type of register set */
        {
        WDB_REG_SET_IU	= 0,            /* integer unit register set */
        WDB_REG_SET_FPU	= 1,            /* floating point unit register set */
        WDB_REG_SET_MMU	= 2,            /* memory management unit reg set */
        WDB_REG_SET_CU	= 3,            /* cache unit register set */
        WDB_REG_SET_TPU	= 4,            /* timer processor unit register set */
        WDB_REG_SET_SYS	= 5,		/* system registers */
        WDB_REG_SET_DSP	= 6,		/* dsp registers */
        WDB_REG_SET_AV	= 7,		/* altivec registers */
        WDB_REG_SET_LAST	= 8		/* 1 + last used REG_SET number */
        } ;

            //WDB_CTX_TYPE				/* type of context on the target */
        {
        WDB_CTX_SYSTEM	= 0000,		/* system mode */
        WDB_CTX_GROUP	= 0001,		/* process group (not implemented) */
        WDB_CTX_ANY		= 0002,		/* any context (not implemented) */
        WDB_CTX_TASK	= 0003,		/* specific task or processes */
        WDB_CTX_ANY_TASK	= 0004,		/* any task */
        WDB_CTX_ISR		= 0005,		/* specific ISR (not implemented) */
        WDB_CTX_ANY_ISR	= 0006		/* any ISR (not implemented) */
        } ;
            //WDB_MEM_REGION			/* a region of target memory */
        {
        TGT_INT_T		options;	/* memory options */
        TGT_ADDR_T		baseAddr;	/* memory region base address */
        TGT_INT_T		numBytes;	/* memory region size */
        UINT32		param;		/* proc dependent parameter */
        } ;

        """
        try:
            #wdb_core
            parameter = parameter + struct.pack('>I', 3)
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0)

            #WDB_REG_READ_DESC
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 3)
            parameter = parameter + struct.pack('>I', 1)
            parameter = parameter + struct.pack('>I', 1)
            parameter = parameter + struct.pack('>I', taskid)

            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0)
            parameter = parameter + struct.pack('>I', 0x28)
            parameter = parameter + struct.pack('>I', 0)

            bok, data = self._Request(0x28, parameter)   #WDB_REGS_GET

            """     output: WDB_MEM_XFER
            {
            TGT_INT_T		options;	/* memory options */
            WDB_OPQ_DATA_T	source;		/* data to transfer */
            TGT_ADDR_T		destination;	/* requested destination */
            TGT_INT_T		numBytes;	/* number of bytes transferred */
            };
            --------
            56b88743
            00000001
            00000000
            00000000
            00000000
            00000000
            fffff87b
            00000054
            00000000
                                            out:WDB_MEM_REGION
            00000000
            00000028
            00000000
            00000028

            01000000		edi
            b4b8f600		esi
            64b8f600		ebp
            44b8f600		esp
            d4b9f600		ebx
            64ddff00		edx
            b4b9f600		ecx
            88888888		eax
            06020000		efalgs
            9c473900		pc
            """
            'edi,esi,ebp,esp,ebx,edx,ecx,eac,efalgs,pc'
            if bok == True:
                outparam = data[0:16]
                data = data[16:]

                options, source,destination,numBytes = struct.unpack(">IIII", outparam)
        except Exception as e:
            bok = False



        return bok, data

    def PushGetExcepData(self):
        bok = False
        data = ''
        parameter = ''
        evecounts = 0

        """     input:
        none
        """
        try:

            bok, data = self._Request(0x46, parameter)   #WDB_EVENT_GET

            """     output:WDB_EVT_DATA
            {
              WDB_EVT_TYPE
              WDB_EVT_INFO
            } ;

                {
                WDB_EVT_NONE	= 0,		/* no event */
                WDB_EVT_CTX_START	= 1,		/* context creation */
                WDB_EVT_CTX_EXIT	= 2,		/* context exit */
                WDB_EVT_BP		= 3,		/* breakpoint */
                WDB_EVT_HW_BP	= 4,		/* hardware breakpoint */
                WDB_EVT_WP		= 5,		/* watchpoint */
                WDB_EVT_EXC		= 6,		/* exception */
                WDB_EVT_VIO_WRITE	= 7,		/* virtual I/O write */
                WDB_EVT_CALL_RET	= 8,		/* function call finished */
                WDB_EVT_TSFS_OP	= 9,		/* VIO II operation */
                WDB_EVT_USER	= 10		/* user event */
                } WDB_EVT_TYPE;


                        /* 0 - 19	exception vector number */

                #define IN_DIVIDE_ERROR			0
                #define IN_DEBUG			1
                #define IN_NON_MASKABLE			2
                #define IN_BREAKPOINT			3
                #define IN_OVERFLOW			4
                #define IN_BOUND			5
                #define IN_INVALID_OPCODE		6
                #define IN_NO_DEVICE			7
                #define IN_DOUBLE_FAULT			8
                #define IN_CP_OVERRUN			9
                #define IN_INVALID_TSS			10
                #define IN_NO_SEGMENT			11
                #define IN_STACK_FAULT			12
                #define IN_PROTECTION_FAULT		13
                #define IN_PAGE_FAULT			14
                #define IN_RESERVED			15
                #define IN_CP_ERROR			16
                #define IN_ALIGNMENT			17
                #define IN_MACHINE_CHECK		18
                #define IN_SIMD				19

            """

            if bok == True and len(data) > 8:
                evetype,evecounts = struct.unpack('>II', data[:8])
                data = data [8:]

                if evetype == 6:
                    bok = True


                else:
                    print "===============================unknown event=======================================\n"
                    print "evetype = %d evecounts = %d " % (evetype, evecounts)
                    i = 0
                    while (i < evecounts):
                        value = struct.unpack('>I', data[i*4:i*4+4])[0]
                        print "array[%d] = %x" % (i, value)
                        i = i + 1
                    print "===================================================================================\n"
                    bok = False
            else:
                bok = False

        except Exception as e:
            bok = False

        return bok, evecounts, data


    def pub_Connect(self):
        bok = False
        data = ''


        bok,data = self.PushConnect(self.gDstip, self.gDstport, self.gOutTime)
        '''
        if bok ==True:
            gThreadRecvExcepEvent = threading.Thread(target = ThreadWork, name = "ThreadWork")

        gThreadRecvExcepEvent.start()
        '''
        return bok

    def pub_DisConnect(self):

        bok = False

        #PushDisConnect()
        bok = self.PushRebootTarget()
        #if self.gSock != None:
            #self.gSock.close()

        #self.gSock = None

        return bok


    def pub_IsAbort(self):

        excepinfo = {}
        excepstr = ['DIVIDE_ERROR', 'DEBUG', 'NON_MASKABLE', 'BREAKPOINT', 'OVERFLOW', 'BOUND', 'INVALID_OPCODE', 'NO_DEVICE', 'DOUBLE_FAULT', 'CP_OVERRUN', 'INVALID_TSS', 'NO_SEGMENT', 'STACK_FAULT', 'PROTECTION_FAULT', 'PAGE_FAULT', 'RESERVED', 'CP_ERROR', 'ALIGNMENT', 'MACHINE_CHECK', 'SIMD']

        eventmap = {}
        regmap = {}
        taskid = 0
        data = ''

        #if gOcuExcepEvent == False:
        #    return False,None


        #get event
        bok,evecount,data = self.PushGetExcepData()
        if bok == False:
            return False,None
        '''
        data --> WDB_EXC_INFO 		/* WDB_EVT_INFO for exceptions */
            {
            UINT32	numLongs;	/* structure length */
            TGT_LONG_T	status;		/* status of context */
            TGT_LONG_T	stoppedCtxType; /* context stopped by exception */
            TGT_LONG_T	stoppedArgs [2];
            TGT_LONG_T	excCtxType;	/* context that got exception */
            TGT_LONG_T	excArgs [2];
            TGT_LONG_T	vec;		/* hardware trap number */
            TGT_ADDR_T	pEsf;		/* address of exception stack frame */
            TGT_LONG_T	sigNum;		/* signal number */
            } ;

        '''

        status,stoppedCtxType,stoppedArgs1,stoppedArgs2,excCtxType,excArgs1,excArgs2,vec,pEsf,sigNum = struct.unpack('>IIIIIIIIII', data)
        taskid = stoppedArgs1

        key = excepstr[vec]
        value = '????????'
        eventmap[key] = value


        #get reg
        if taskid != 0:
            bok, data = self.PushReadRegister(taskid)
        if bok == False:
            return bok, None

        #gByteBigorder == false
        edi,esi,ebp,esp,ebx,edx,ecx,eax,eflags,pc = struct.unpack('<IIIIIIIIII', data)
        regmap['edi'] = edi
        regmap['esi'] = esi
        regmap['ebp'] = ebp
        regmap['esp'] = esp
        regmap['ebx'] = ebx
        regmap['edx'] = edx
        regmap['ecx'] = ecx
        regmap['eax'] = eax
        regmap['eflags'] = eflags
        regmap['pc'] = pc

        #get code
        bok,data = self.PushReadMemory(pc, 100)
        asmcode = ''

        #depend capstone
        #http://www.capstone-engine.org/download.html
        #Python module for Windows 32 - Binaries
        if bok == True and len(data) > 0:
            md = Cs(CS_ARCH_X86, CS_MODE_32)
            for i in md.disasm(data, pc):
                asmcode =  asmcode + ("0x%x:\t%s\t%s\n" % (i.address, i.mnemonic, i.op_str))

        excepinfo['eve'] = eventmap
        excepinfo['reg'] = regmap
        excepinfo['asm'] = asmcode

        return bok, excepinfo




def test():
    bok = False
    target =  VXWORKS66('192.168.102.89', doChksum=False)
    info = {}

    bok = target.pub_Connect()
    bok, data = target._Request(RId=123, parameter='', bRecvData=True)

    while 1:
        bok,info = target.pub_IsAbort()
        time.sleep(1)

        if bok :
            print "%s" % info['asm']
            break



    target.pub_DisConnect()


if __name__ == "__main__":

    print "game runing...................\n"
    test()
    print "game over!!!!!!!!!!!!!!!!!!!!"
